// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Ability/ARTGlobalTags.h"
#include "AI/Order/ARTOrderHelper.h"
#include "AI/Order/ARTSelectComponent.h"
#include "ARTCharacter/AI/ARTAIController.h"
#include "Net/UnrealNetwork.h"
#include "ART/ART.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

// Sets default values for this component's properties
UARTOrderComponent::UARTOrderComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game staART, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//SetIsReplicated(true);

	LastOrderHomeLocation = FVector::ZeroVector;
	bIsHomeLocationSet = false;
	// ...
}

void UARTOrderComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn == nullptr)
	{
		return;
	}

	// showing order previews for selected units.
	//// Register for selection events.
	SelectComponent = Pawn->FindComponentByClass<UARTSelectComponent>();

	if (SelectComponent != nullptr)
	{
		SelectComponent->OnSelected.AddDynamic(this, &UARTOrderComponent::OnSelected);
		SelectComponent->OnDeselected.AddDynamic(this, &UARTOrderComponent::OnDeselected);
	}

	// Reset current order.
	CurrentOrder = FARTOrderData();

	// Try to set the stop order if possible.
	AARTAIController* Controller = Cast<AARTAIController>(Pawn->GetController());
	if (Controller == nullptr)
	{
		return;
	}

	StopOrder = Controller->GetStopOrder();

	CurrentOrder = StopOrder;
	IssueOrder(StopOrder);
}

void UARTOrderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARTOrderComponent, CurrentOrder);
	DOREPLIFETIME(UARTOrderComponent, OrderQueue);
}

void UARTOrderComponent::SetCurrentOrder(FARTOrderData NewOrder)
{
	LastOrder = CurrentOrder;

	CurrentOrder = NewOrder;

	NotifyOnOrderChanged(CurrentOrder);
}

void UARTOrderComponent::NotifyOnOrderChanged(const FARTOrderData& NewOrder)
{
	UpdateOrderPreviews();

	OnOrderChanged.Broadcast(NewOrder);
}

void UARTOrderComponent::ReceivedCurrentOrder()
{
	NotifyOnOrderChanged(CurrentOrder);
}

void UARTOrderComponent::ReceivedOrderQueue()
{
	UpdateOrderPreviews();
}

void UARTOrderComponent::IssueOrder(const FARTOrderData& Order)
{
	AActor* Owner = GetOwner();

	// It is impossible for clients to issue orders. Clients need to issue orders using their player controller.
	if (!Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Error,
		       TEXT("The order %s was send from a client. It is impossible for clients to issue orders. Clients need "
			       "to issue orders using their player controller. "),
		       *Order.ToString());
		return;
	}

	// Clear the order cue when another order is issued.
	OrderQueue.Empty();
	OnOrderQueueCleared.Broadcast();

	// Do nothing if we are obeying exact the same order already (and I mean exact: Not only the same order type)
	if (CurrentOrder == Order)
	{
		return;
	}

	if (!Order.OrderType.IsValid())
	{
		Order.OrderType.LoadSynchronous();
	}

	bIsHomeLocationSet = false;

	// Abort current order.
	if (CurrentOrder.OrderType != StopOrder &&
		UARTOrderHelper::GetOrderProcessPolicy(Order.OrderType, Owner, Order.OrderTags, Order.Index) !=
		EARTOrderProcessPolicy::INSTANT)
	{
		switch (UARTOrderHelper::GetOrderProcessPolicy(CurrentOrder.OrderType, Owner, CurrentOrder.OrderTags,
		                                               CurrentOrder.Index))
		{
		case EARTOrderProcessPolicy::CAN_BE_CANCELED:
			OrderCanceled();

			if (CheckOrder(Order))
			{
				ObeyOrder(Order);
			}
			else
			{
				//ObeyStopOrder();
			}
			break;
		case EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED:
			// We cannot cancel our current order so we need to queue it up as next.
			OrderQueue.Add(Order);
			break;
		case EARTOrderProcessPolicy::INSTANT:
			// This should not be possible. Instant orders should not be set as current orders in the first place.
			check(0);
			break;
		default:
			check(0);
			break;
		}
	}

	else
	{
		if (CheckOrder(Order))
		{
			ObeyOrder(Order);
		}
		else
		{
			ObeyStopOrder();
		}
	}
}

void UARTOrderComponent::ClearOrderQueue()
{
	AActor* Owner = GetOwner();

	// It is impossible for clients to issue orders. Clients need to issue orders using their player controller.
	if (!Owner->HasAuthority())
	{
		UE_LOG(LogOrder, Error,
		       TEXT("It is impossible for clients to issue orders. Clients need "
			       "to issue orders using their player controller. "));
		return;
	}

	// Clear the order cue when another order is issued.
	OrderQueue.Empty();
	OnOrderQueueCleared.Broadcast();
}

void UARTOrderComponent::EnqueueOrder(const FARTOrderData& Order)
{
	// It is impossible for clients to issue orders. Clients need to issue orders using their player controller.
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogOrder, Error,
		       TEXT("The order %s was send from a client. It is impossible for clients to issue orders. Clients need "
			       "to issue orders using their player controller. "),
		       *Order.ToString());
		return;
	}

	if (!CheckOrder(Order))
	{
		return;
	}

	if (OrderQueue.Num() == 0 && CurrentOrder.OrderType == StopOrder)
	{
		ObeyOrder(Order);
	}

	else
	{
		// Do nothing if we are obeying exact the same order already (and I mean exact: Not only the same order type)
		if (OrderQueue.Num() > 0 && OrderQueue.Last() == Order)
		{
			return;
		}

		if(OrderQueue.Num() > 0 && OrderQueue.Last() == StopOrder)
		{
			OrderQueue.RemoveAt(OrderQueue.Num()-1);
		}

		OrderQueue.Add(Order);
		OnOrderEnqueued.Broadcast(Order);

		UpdateOrderPreviews();
	}
}

void UARTOrderComponent::InsertOrderAfterCurrentOrder(const FARTOrderData& Order)
{
	// It is impossible for clients to issue orders. Clients need to issue orders using their player controller.
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogOrder, Error,
		       TEXT("The order %s was send from a client. It is impossible for clients to issue orders. Clients need "
			       "to issue orders using their player controller. "),
		       *Order.ToString());
		return;
	}

	if (!CheckOrder(Order))
	{
		return;
	}

	if (OrderQueue.Num() == 0 && CurrentOrder.OrderType == StopOrder)
	{
		ObeyOrder(Order);
	}

	bIsHomeLocationSet = false;

	OrderQueue.Insert(Order, 0);
}

void UARTOrderComponent::InsertOrderBeforeCurrentOrder(const FARTOrderData& Order)
{
	// It is impossible for clients to issue orders. Clients need to issue orders using their player controller.
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogOrder, Error,
		       TEXT("The order %s was send from a client. It is impossible for clients to issue orders. Clients need "
			       "to issue orders using their player controller. "),
		       *Order.ToString());
		return;
	}

	if (!CheckOrder(Order))
	{
		return;
	}

	// Queue the current order.
	OrderQueue.Insert(CurrentOrder, 0);

	// Save home location of the current order.
	AARTAIController* Controller = Cast<AARTAIController>(Cast<APawn>(GetOwner())->GetController());
	if (Controller != nullptr)
	{
		LastOrderHomeLocation = Controller->GetHomeLocation();
	}

	// Directly obey the passed order.
	ObeyOrder(Order);

	// Set to true after obey order to not use the last order home location for the current order.
	bIsHomeLocationSet = true;
}

TSoftClassPtr<UARTOrder> UARTOrderComponent::GetCurrentOrderType() const
{
	return CurrentOrder.OrderType;
}

bool UARTOrderComponent::IsIdle() const
{
	return GetCurrentOrderType() == StopOrder;
}

FARTOrderData UARTOrderComponent::GetCurrentOrderData() const
{
	return CurrentOrder;
}

TArray<FARTOrderData> UARTOrderComponent::GetCurrentOrderDataQueue() const
{
	return OrderQueue;
}

AActor* UARTOrderComponent::GetCurrentOrderTargetActor() const
{
	return CurrentOrder.Target;
}

FVector2D UARTOrderComponent::GetCurrentOrderTargetLocation() const
{
	return CurrentOrder.Location;
}

int32 UARTOrderComponent::GetCurrentOrderTargetIndex() const
{
	return CurrentOrder.Index;
}

FGameplayTagContainer UARTOrderComponent::GetCurrentOrderTagContainer() const
{
	return CurrentOrder.OrderTags;
}

void UARTOrderComponent::ObeyOrder(const FARTOrderData& Order)
{
	AActor* Owner = GetOwner();
	FARTOrderTargetData TargetData = UARTOrderHelper::CreateOrderTargetData(Owner, Order.Target, Order.Location);

	// Find the correct home location value for this order.
	FVector HomeLocation;
	if (bIsHomeLocationSet)
	{
		HomeLocation = LastOrderHomeLocation;
		bIsHomeLocationSet = false;
	}

	else
	{
		HomeLocation = Owner->GetActorLocation();
	}

	switch (UARTOrderHelper::GetOrderProcessPolicy(Order.OrderType, Owner, Order.OrderTags, Order.Index))
	{
	case EARTOrderProcessPolicy::INSTANT:
		{
			// An instant order is issued directly without changing the current order (ProductionOrder,
			// SetRallyPointOrder,
			// Some Abilities etc.).
			// Note: It is currently not possible to queue instant order because of the missing callback.
			// Maybe 'ObeyOrder' needs a return value that describes if the order is in progress or finished.
			Order.OrderType->GetDefaultObject<UARTOrder>()->IssueOrder(Owner, TargetData, Order.OrderTags, Order.Index,
			                                                           FARTOrderCallback(), HomeLocation);
		}
		break;
	case EARTOrderProcessPolicy::CAN_BE_CANCELED:
	case EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED:
		{
			if (CurrentOrder.OrderType != StopOrder)

			{
				UnregisterTagListeners(CurrentOrder);
			}

			SetCurrentOrder(Order);

			FARTOrderCallback Callback;
			Callback.AddDynamic(this, &UARTOrderComponent::OnOrderEndedCallback);

			if (!Order.OrderType.IsValid())
			{
				Order.OrderType.LoadSynchronous();
			}

			if (Order.OrderType != StopOrder)
			{
				RegisterTagListeners(Order);
			}

			Order.OrderType->GetDefaultObject<UARTOrder>()->IssueOrder(Owner, TargetData, Order.OrderTags, Order.Index,
			                                                           Callback,
			                                                           HomeLocation);
		}
		break;
	default:
		check(0);
		break;
	}
}

bool UARTOrderComponent::CheckOrder(const FARTOrderData& Order) const
{
	FARTOrderErrorTags OrderErrorTags;

	AActor* OrderedActor = GetOwner();
	if (!Order.OrderType.IsValid())
	{
		Order.OrderType.LoadSynchronous();
	}

	TSubclassOf<UARTOrder> OrderType = Order.OrderType.Get();
	if (OrderType == nullptr)
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderComponent::CheckOrder: The specified order for the actor '%s' is invalid."),
		       *OrderedActor->GetName());
		return false;
	}

	if (!UARTOrderHelper::CanObeyOrder(OrderType.Get(), OrderedActor, Order.OrderTags, Order.Index, &OrderErrorTags))
	{
		LogOrderErrorMessage(
			FString::Printf(TEXT("UARTOrderComponent::CheckOrder: The actor '%s' cannot obey the order '%s'."),
			                *OrderedActor->GetName(), *OrderType->GetName()),
			OrderErrorTags);
		return false;
	}

	FARTOrderTargetData TargetData = UARTOrderHelper::CreateOrderTargetData(OrderedActor, Order.Target, Order.Location);
	if (!UARTOrderHelper::IsValidTarget(OrderType.Get(), OrderedActor, TargetData, Order.OrderTags, Order.Index,
	                                    &OrderErrorTags))
	{
		LogOrderErrorMessage(
			FString::Printf(
				TEXT("UARTOrderComponent::CheckOrder: The actor '%s' was issued to obey the order '%s', but the "
					"target data is invalid: %s"),
				*OrderedActor->GetName(), *OrderType->GetName(), *TargetData.ToString()),
			OrderErrorTags);
		return false;
	}

	return true;
}

void UARTOrderComponent::LogOrderErrorMessage(const FString& Message, const FARTOrderErrorTags& OrderErrorTags) const
{
	// TODO: Better formatting?
	FString FinalMessage = Message + TEXT(": ") + OrderErrorTags.ToString();
	UE_LOG(LogOrder, Warning, TEXT("%s"), *FinalMessage);
}

void UARTOrderComponent::OnOrderEndedCallback(EARTOrderResult OrderResult)
{
	OrderEnded(OrderResult);
}

void UARTOrderComponent::OrderEnded(EARTOrderResult OrderResult)
{
	AActor* Owner = GetOwner();

	switch (OrderResult)
	{
	case EARTOrderResult::FAILED:
		if (StopOrder != nullptr)
		{
			// OrderCanceled will be raised in IssueOrder.
			IssueOrder(FARTOrderData(StopOrder));
		}
		return;
	case EARTOrderResult::CANCELED:
		if (!UARTOrderHelper::CanOrderBeConsideredAsSucceeded(
			CurrentOrder.OrderType, Owner,
			UARTOrderHelper::CreateOrderTargetData(Owner, CurrentOrder.Target, CurrentOrder.Location),
			CurrentOrder.OrderTags, CurrentOrder.Index))
		{
			// OrderCanceled will be raised in IssueOrder.
			IssueOrder(FARTOrderData(StopOrder));
			break;
		}
		// Fall through if succeeded
	case EARTOrderResult::SUCCEEDED:
		if (OrderQueue.IsValidIndex(0))
		{
			const FARTOrderData NewOrder = OrderQueue[0];

			if (CheckOrder(NewOrder))
			{
				OrderQueue.RemoveAt(0);
				//TODO: make movement order smooth by not reseting the BT
				//ObeyStopOrder();
				ObeyOrder(NewOrder);
				return;
			}

			OrderQueue.Empty();
			ObeyStopOrder();
		}

		else if (StopOrder != nullptr)
		{
			ObeyStopOrder();
		}

		return;
	}
}

void UARTOrderComponent::OrderCanceled()
{
	AActor* Owner = GetOwner();
	FARTOrderTargetData TargetData =
		UARTOrderHelper::CreateOrderTargetData(Owner, CurrentOrder.Target, CurrentOrder.Location);

	UClass* OrderType = nullptr;

	if (!CurrentOrder.OrderType.IsValid())
	{
		OrderType = CurrentOrder.OrderType.LoadSynchronous();
	}
	else
	{
		OrderType = CurrentOrder.OrderType.Get();
	}

	if (OrderType != nullptr)
	{
		OrderType->GetDefaultObject<UARTOrder>()->OrderCanceled(Owner, TargetData, CurrentOrder.OrderTags,
		                                                        CurrentOrder.Index);
	}
}

void UARTOrderComponent::RegisterTagListeners(const FARTOrderData& Order)
{
	AActor* Owner = GetOwner();
	FARTOrderTagRequirements TagRequirements;
	UARTOrderHelper::GetOrderTagRequirements(Order.OrderType, Owner, Order.OrderTags, Order.Index, TagRequirements);

	UAbilitySystemComponent* OwnerAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	// Owner tags
	//

	if (OwnerAbilitySystem != nullptr)
	{
		FGameplayTagContainer OwnerTags;
		for (FGameplayTag Tag : TagRequirements.SourceRequiredTags)
		{
			// Don't register a delegate for permanent status tags.
			/*if (!Tag.MatchesTag(UARTGlobalTags::Status_Permanent()))
			{
				OwnerTags.AddTagFast(Tag);
			}*/
		}

		for (FGameplayTag Tag : TagRequirements.SourceBlockedTags)
		{
			// Don't register a delegate for permanent status tags.
			/*if (!Tag.MatchesTag(UARTGlobalTags::Status_Permanent()))
			{
				OwnerTags.AddTag(Tag);
			}*/
		}

		// TODO: Hard coded check for visibility change. Is their a more generic way todo this?
		/*if (TagRequirements.TargetRequiredTags.HasTag(UARTGlobalTags::Relationship_Visible()))
		{
			OwnerTags.AddTag(UARTGlobalTags::Status_Changing_Detector());
		}*/

		// Register a callback for each of the tags to check if it was added to or removed.
		for (FGameplayTag Tag : OwnerTags)
		{
			FOnGameplayEffectTagCountChanged& Delegate =
				OwnerAbilitySystem->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved);

			FDelegateHandle DelegateHandle = Delegate.AddUObject(this, &UARTOrderComponent::OnOwnerTagsChanged);
			RegisteredOwnerTagEventHandles.Add(Tag, DelegateHandle);
		}
	}

	// Target tags
	//

	EARTTargetType TargetType = UARTOrderHelper::GetTargetType(Order.OrderType, Owner, Order.OrderTags, Order.Index);
	if (TargetType == EARTTargetType::ACTOR)
	{
		UAbilitySystemComponent* TargetAbilitySystem =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Order.Target);

		if (TargetAbilitySystem != nullptr)
		{
			FGameplayTagContainer TargetTags;
			for (FGameplayTag Tag : TagRequirements.TargetRequiredTags)
			{
				// Don't register a delegate for permanent status tags.
				/*if (!Tag.MatchesTag(UARTGlobalTags::Status_Permanent()))
				{
					TargetTags.AddTagFast(Tag);
				}*/
			}

			for (FGameplayTag Tag : TagRequirements.TargetBlockedTags)
			{
				// Don't register a delegate for permanent status tags.
				/*if (!Tag.MatchesTag(UARTGlobalTags::Status_Permanent()))
				{
					TargetTags.AddTag(Tag);
				}*/
			}

			// TODO: Hard coded check for visibility change. Is their a more generic way todo this?
			/*if (TagRequirements.TargetRequiredTags.HasTag(UARTGlobalTags::Relationship_Visible()))
			{
				TargetTags.AddTag(UARTGlobalTags::Status_Changing_Stealthed());
			}*/

			// Register a callback for each of the tags to check if it was added to or removed.
			for (FGameplayTag Tag : TargetTags)
			{
				FOnGameplayEffectTagCountChanged& Delegate =
					TargetAbilitySystem->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved);

				FDelegateHandle DelegateHandle = Delegate.AddUObject(this, &UARTOrderComponent::OnTargetTagsChanged);
				RegisteredTargetTagEventHandles.Add(Tag, DelegateHandle);
			}
		}
	}
}

void UARTOrderComponent::UnregisterTagListeners(const FARTOrderData& Order)
{
	AActor* Owner = GetOwner();
	FARTOrderTagRequirements TagRequirements;
	UARTOrderHelper::GetOrderTagRequirements(Order.OrderType, Owner, Order.OrderTags, Order.Index, TagRequirements);

	UAbilitySystemComponent* OwnerAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	// Owner tags
	//

	if (OwnerAbilitySystem != nullptr)
	{
		for (TPair<FGameplayTag, FDelegateHandle> Pair : RegisteredOwnerTagEventHandles)
		{
			FOnGameplayEffectTagCountChanged& Delegate =
				OwnerAbilitySystem->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved);

			Delegate.Remove(Pair.Value);
		}

		RegisteredOwnerTagEventHandles.Empty();
	}

	// Target tags
	//

	EARTTargetType TargetType = UARTOrderHelper::GetTargetType(Order.OrderType, Owner, Order.OrderTags, Order.Index);
	if (TargetType == EARTTargetType::ACTOR)
	{
		UAbilitySystemComponent* TargetAbilitySystem =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Order.Target);

		if (TargetAbilitySystem != nullptr)
		{
			for (TPair<FGameplayTag, FDelegateHandle> Pair : RegisteredTargetTagEventHandles)
			{
				FOnGameplayEffectTagCountChanged& Delegate =
					TargetAbilitySystem->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved);

				Delegate.Remove(Pair.Value);
			}

			RegisteredTargetTagEventHandles.Empty();
		}
	}
}

void UARTOrderComponent::OnTargetTagsChanged(const FGameplayTag Tag, int32 NewCount)
{
	FARTOrderTagRequirements TagRequirements;
	UARTOrderHelper::GetOrderTagRequirements(CurrentOrder.OrderType, GetOwner(), CurrentOrder.OrderTags,
	                                         CurrentOrder.Index, TagRequirements);

	if ((NewCount && TagRequirements.TargetBlockedTags.HasTag(Tag)) ||
		(!NewCount && TagRequirements.TargetRequiredTags.HasTag(Tag)))
	{
		OrderEnded(EARTOrderResult::CANCELED);
	}

	// TODO: Hard coded check for visibility change. Is their a more generic way todo this?
	/*else if (NewCount && Tag == UARTGlobalTags::Status_Changing_Stealthed() &&
		TagRequirements.TargetRequiredTags.HasTag(UARTGlobalTags::Behaviour_Visible())
	{
		if (!UARTBlueprintFunctionLibrary::IsVisibleForActor(GetOwner(), CurrentOrder.Target))
		{
			OrderEnded(EARTOrderResult::CANCELED);
		}
	}*/
}

void UARTOrderComponent::OnOwnerTagsChanged(const FGameplayTag Tag, int32 NewCount)
{
	FARTOrderTagRequirements TagRequirements;
	UARTOrderHelper::GetOrderTagRequirements(CurrentOrder.OrderType, GetOwner(), CurrentOrder.OrderTags,
	                                         CurrentOrder.Index, TagRequirements);

	if ((NewCount && TagRequirements.SourceBlockedTags.HasTag(Tag)) ||
		!NewCount && TagRequirements.SourceRequiredTags.HasTag(Tag))
	{
		OrderEnded(EARTOrderResult::CANCELED);
	}

	// TODO: Hard coded check for visibility change. Is their a more generic way todo this?
	/*else if (!NewCount && Tag == UARTGlobalTags::Status_Changing_Detector() &&
		TagRequirements.TargetRequiredTags.HasTag(UARTGlobalTags::Relationship_Visible()))
	{
		if (!UARTBlueprintFunctionLibrary::IsVisibleForActor(GetOwner(), CurrentOrder.Target))
		{
			OrderEnded(EARTOrderResult::CANCELED);
		}
	}*/
}

void UARTOrderComponent::ObeyStopOrder()
{
	if (StopOrder == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("%s has no stop order set."), *GetOwner()->GetName());
		return;
	}

	if (!StopOrder.IsValid())
	{
		StopOrder.LoadSynchronous();
	}

	ObeyOrder(FARTOrderData(StopOrder));
}

AActor* UARTOrderComponent::CreateOrderPreviewActor(const FARTOrderData& Order)
{
	FARTOrderPreviewData PreviewData = UARTOrderHelper::GetOrderPreviewData(Order.OrderType, GetOwner(), Order.OrderTags, Order.Index);
	if(!PreviewData.GetOrderPreviewClass()) return nullptr;
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Cast<APawn>(GetOwner());
	SpawnInfo.ObjectFlags |= RF_Transient;

	FVector Location;
	AARTOrderPreview* SpawnedActor = nullptr;

	// Spawn default order preview.
	EARTTargetType TargetType = UARTOrderHelper::GetTargetType(Order.OrderType, GetOwner(),
	                                                           Order.OrderTags);
	
	
	switch (TargetType)
	{
	case EARTTargetType::NONE:
		break;
	case EARTTargetType::ACTOR:
		if (Order.Target != nullptr)
		{
			Location = Order.Target->GetActorLocation();
			SpawnedActor =
				GetWorld()->SpawnActor<AARTOrderPreview>(PreviewData.GetOrderPreviewClass(), Location, FRotator::ZeroRotator, SpawnInfo);

			SpawnedActor->AttachToActor(Order.Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			OrderPreviewActors.Add(SpawnedActor);
		}
		break;
	case EARTTargetType::LOCATION:
		Location = FVector(Order.Location.X, Order.Location.Y, 0);
		SpawnedActor =
			GetWorld()->SpawnActor<AARTOrderPreview >(PreviewData.GetOrderPreviewClass(), Location, FRotator::ZeroRotator, SpawnInfo);
		OrderPreviewActors.Add(SpawnedActor);
		break;
	case EARTTargetType::DIRECTION:
		// NOTE(np): In A Year Of Rain, we're using a raycast to translate between 3D and 2D space.
		//Location = UARTUtilities::GetGroundLocation2D(this, Order.Location);
		Location = FVector(Order.Location.X, Order.Location.Y, 0.0f);
		SpawnedActor =
			GetWorld()->SpawnActor<AARTOrderPreview>(PreviewData.GetOrderPreviewClass(), Location, FRotator::ZeroRotator, SpawnInfo);
		OrderPreviewActors.Add(SpawnedActor);
		break;
	case EARTTargetType::PASSIVE:
		break;
	default:
		break;
	}

	if(SpawnedActor)
	{
		FARTOrderTargetData TargetData = UARTOrderHelper::CreateOrderTargetData(GetOwner(), Order.Target, Order.Location);
		UARTOrderHelper::InitializePreviewActor(Order.OrderType, SpawnedActor, GetOwner(), TargetData, Order.OrderTags, Order.Index);
	}

	return SpawnedActor;
}

void UARTOrderComponent::OnSelected()
{
	UpdateOrderPreviews();
}

void UARTOrderComponent::OnDeselected()
{
	// Destroy previews except for construction previews.
	for (int32 Index = OrderPreviewActors.Num() - 1; Index > 0; --Index)
	{
		if (OrderPreviewActors.IsValidIndex(Index) && OrderQueue.IsValidIndex(Index - 1) &&
			OrderQueue[Index - 1].OrderType != BeginConstructionOrder)
		{
			OrderPreviewActors[Index]->Destroy();
			OrderPreviewActors.RemoveAt(Index);
		}
	}

	if (OrderPreviewActors.IsValidIndex(0) && CurrentOrder.OrderType != BeginConstructionOrder)
	{
		OrderPreviewActors[0]->Destroy();
		OrderPreviewActors.RemoveAt(0);
	}
}

void UARTOrderComponent::UpdateOrderPreviews()
{
	// Clear old previews.
	for (AActor* OrderPreviewActor : OrderPreviewActors)
	{
		if (IsValid(OrderPreviewActor))
		{
			OrderPreviewActor->Destroy();
		}
	}

	OrderPreviewActors.Empty();

	// Don't spawn order previews for actors of others.
	AActor* Owner = GetOwner();

	if (!IsValid(Owner))
	{
		return;
	}

	// Spawn new previews.
	bool bSelected = SelectComponent != nullptr && SelectComponent->IsSelected();

	/*if ((bSelected && OrderQueue.Num() > 0) || CurrentOrder.OrderType == BeginConstructionOrder)
	{
	CreateOrderPreviewActor(CurrentOrder);
	}*

	for (const FARTOrderData& OrderData : OrderQueue)
	{
		if (bSelected || OrderData.OrderType == BeginConstructionOrder)
		{
			CreateOrderPreviewActor(OrderData);
		}
	}*/

	if (bSelected && CurrentOrder != StopOrder)
	{
		CreateOrderPreviewActor(CurrentOrder);
	}
}

// Called every frame
void UARTOrderComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
