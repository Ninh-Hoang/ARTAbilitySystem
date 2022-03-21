// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Order/ARTUseAbilityOrder.h"

#include "Ability/ARTGameplayAbility_Order.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

UARTUseAbilityOrder::UARTUseAbilityOrder()
{
}

bool UARTUseAbilityOrder::CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                                       FARTOrderErrorTags* OutErrorTags /*= nullptr*/) const
{
	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	if(!AbilitySystem) return false;

	TArray<FGameplayAbilitySpec*> SpecArray;
	//only get activatable ability
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, true);
	if (SpecArray.Num() > 0)
	{
		FGameplayAbilitySpec* Spec = SpecArray[0];
		UGameplayAbility* CDOAbility = Spec->Ability;
		UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();
		
		UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : CDOAbility;
		
		if(AbilitySource)
		{
			// Check if ability has been learned yet.
			/*if (Spec.Level <= 0)
			{
				return false;
			}*/
			
			FGameplayTagContainer FailureTags;

			// Don't pass any source and target tags to can activate ability. These tags has already been checked in
			// 'UARTOrderHelper'. Only the activation required and activation blocked tags are checked here.
			if (!AbilitySource->CanActivateAbility(Spec->Handle, AbilitySystem->AbilityActorInfo.Get(), nullptr, nullptr,
											 &FailureTags))
			{
				if (OutErrorTags != nullptr)
				{
					OutErrorTags->ErrorTags = FailureTags;
				}

				return false;
			}
			return true;
		}
	}
	return false;
}

EARTTargetType UARTUseAbilityOrder::GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                  int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return EARTTargetType::NONE;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetTargetType();
	}

	return EARTTargetType::NONE;
}

void UARTUseAbilityOrder::IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                     const FGameplayTagContainer& OrderTags, int32 Index,
                                     FARTOrderCallback Callback, const FVector& HomeLocation)
{
	if (OrderedActor == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("Ordered actor is invalid."));
		Callback.Broadcast(EARTOrderResult::FAILED);
		return;
	}


	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UGameplayAbility* Ability = GetAbility(AbilitySystem, OrderTags);

	if (GetOrderProcessPolicy(OrderedActor, OrderTags, Index) == EARTOrderProcessPolicy::INSTANT)
	{
		if (AbilitySystem->TryActivateAbilityByClass(Ability->GetClass()))
		{
			Callback.Broadcast(EARTOrderResult::SUCCEEDED);
		}
		else
		{
			Callback.Broadcast(EARTOrderResult::FAILED);
		}
		/*FGameplayEventData EventData;
		UARTAbilitySystemHelper::CreateGameplayEventData(OrderedActor, TargetData, Ability->GetClass(), EventData);

		int32 TriggeredAbilities = UARTAbilitySystemHelper::SendGameplayEvent(OrderedActor, EventData);
		if (TriggeredAbilities > 0)
		{
			Callback.Broadcast(EARTOrderResult::SUCCEEDED);
		}

		else
		{
			Callback.Broadcast(EARTOrderResult::FAILED);
		}*/
	}

	else
	{
		Super::IssueOrder(OrderedActor, TargetData, OrderTags, Index, Callback, HomeLocation);
	}
}

UTexture2D* UARTUseAbilityOrder::GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                               int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

FText UARTUseAbilityOrder::GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                   int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'OrderedActor' was 'nullptr'."));
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'Index' was invalid."));
	}

	return Ability->GetAbilityName();
}

FText UARTUseAbilityOrder::GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                          int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FText::FromString(
			TEXT("UARTUseAbilityOrder::GetDescription: Error: Parameter 'OrderedActor' was 'nullptr'."));
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'Index' was invalid."));
	}

	return Ability->GetDescription(OrderedActor);
}

FARTOrderPreviewData UARTUseAbilityOrder::GetOrderPreviewData(const AActor* OrderedActor,
                                                              const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FARTOrderPreviewData();
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FARTOrderPreviewData();
	}

	return Ability->GetAbilityPreviewData();
}

EARTOrderProcessPolicy UARTUseAbilityOrder::GetOrderProcessPolicy(const AActor* OrderedActor,
                                                                  const FGameplayTagContainer& OrderTags,
                                                                  int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return Super::GetOrderProcessPolicy(OrderedActor, OrderTags, Index);
	}

	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	if(!AbilitySystem) return Super::GetOrderProcessPolicy(OrderedActor, OrderTags, Index);
	
	TArray<FGameplayAbilitySpec*> SpecArray;
	FGameplayAbilitySpec* Spec = nullptr;
	UARTGameplayAbility_Order* Ability = nullptr;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, false);
	if (SpecArray.Num() > 0)
	{
		Spec = SpecArray[0];
		UGameplayAbility* CDOAbility = Spec->Ability;
		UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();
		
		UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : CDOAbility;
		
		Ability = Cast<UARTGameplayAbility_Order>(AbilitySource);
	}
	
	if (!Spec || !Ability) return Super::GetOrderProcessPolicy(OrderedActor, OrderTags, Index);

	EAbilityProcessPolicy AbilityProcessPolicy = Ability->GetAbilityProcessPolicy();

	// If this ability does not have a target type or location and is instant we can execute it here directly without
	// altering the AI behavior.
	if ((Ability->GetTargetType() == EARTTargetType::NONE || Ability->GetTargetType() == EARTTargetType::PASSIVE) &&
		AbilityProcessPolicy == EAbilityProcessPolicy::INSTANT)
	{
		return EARTOrderProcessPolicy::INSTANT;
	}

	if (AbilityProcessPolicy == EAbilityProcessPolicy::CAN_NOT_BE_CANCELED)
	{
		return EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED;
	}

	if (AbilityProcessPolicy == EAbilityProcessPolicy::CAN_BE_CANCELED)
	{
		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	if (AbilityProcessPolicy == EAbilityProcessPolicy::CAN_BE_CANCELED_WHEN_NO_GAMEPLAY_TASK_IS_RUNNING)
	{
		if (Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// It should not be possible to run ability tasks using a non instanced ability.
			return EARTOrderProcessPolicy::CAN_BE_CANCELED;
		}

		// Iterate through every active instance to check if any of them has an active ability task.
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : AbilityInstances)
		{
			UARTGameplayAbility_Order* ARTAbilityInstance = Cast<UARTGameplayAbility_Order>(AbilityInstance);

			if (ARTAbilityInstance != nullptr && ARTAbilityInstance->AreAbilityTasksActive())
			{
				return EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED;
			}
		}

		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	return EARTOrderProcessPolicy::CAN_BE_CANCELED;
}

EARTOrderGroupExecutionType UARTUseAbilityOrder::GetGroupExecutionType(const AActor* OrderedActor,
                                                                       const FGameplayTagContainer& OrderTags,
                                                                       int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return EARTOrderGroupExecutionType::ALL;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetGroupExecutionType();
	}

	return EARTOrderGroupExecutionType::ALL;
}

bool UARTUseAbilityOrder::IsHumanPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->IsHumanPlayerAutoAbility();
	}

	return false;
}

bool UARTUseAbilityOrder::GetHumanPlayerAutoOrderInitialState(const AActor* OrderedActor,
                                                              const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetHumanPlayerAutoAutoAbilityInitialState();
	}

	return false;
}

bool UARTUseAbilityOrder::IsAIPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                              int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->IsAIPlayerAutoAbility();
	}

	return false;
}

bool UARTUseAbilityOrder::GetAcquisitionRadiusOverride(const AActor* OrderedActor,
                                                       const FGameplayTagContainer& OrderTags, int32 Index,
                                                       float& OutAcquisitionRadius) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetAcquisitionRadiusOverride(OutAcquisitionRadius);
	}

	return false;
}

void UARTUseAbilityOrder::GetTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                             int32 Index,FARTOrderTagRequirements& OutTagRequirements) const
{
	if (OrderedActor == nullptr)
	{
		return;
	}
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetOrderTagRequirements(OutTagRequirements);
	}
}

float UARTUseAbilityOrder::GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                          const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (!OrderedActor) return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
	
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	if(!AbilitySystem) return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);

	TArray<FGameplayAbilitySpec*> SpecArray;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, false);
	if (SpecArray.Num() > 0)
	{
		FGameplayAbilitySpec* Spec = SpecArray[0];
		UGameplayAbility* CDOAbility = Spec->Ability;
		UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();
		
		UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : CDOAbility;
		
		if(UARTGameplayAbility_Order* ARTAbility =  Cast<UARTGameplayAbility_Order>(AbilitySource))
		{
			return ARTAbility->GetRange(AbilitySystem->AbilityActorInfo.Get(), Spec->Handle);
		}
	}
	return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
}

float UARTUseAbilityOrder::GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                            int32 Index) const
{
	if (OrderedActor == nullptr) return Super::GetRequiredRange(OrderedActor, OrderTags, Index);
	
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	if(!AbilitySystem) return Super::GetRequiredRange(OrderedActor, OrderTags, Index);

	TArray<FGameplayAbilitySpec*> SpecArray;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, false);
	if (SpecArray.Num() > 0)
	{
		FGameplayAbilitySpec* Spec = SpecArray[0];
		UGameplayAbility* CDOAbility = Spec->Ability;
		UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();
		
		UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : CDOAbility;
		
		if(UARTGameplayAbility_Order* ARTAbility =  Cast<UARTGameplayAbility_Order>(AbilitySource))
		{
			return ARTAbility->GetRange(AbilitySystem->AbilityActorInfo.Get(), Spec->Handle);
		}
	}
	return Super::GetRequiredRange(OrderedActor, OrderTags, Index);
}

void UARTUseAbilityOrder::InitializePreviewActor(AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
                                                 const FARTOrderTargetData& TargetData,
                                                 const FGameplayTagContainer& OrderTags, int32 Index)
{
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UGameplayAbility* Ability = GetAbility(AbilitySystem, OrderTags);
	PreviewActor->SetInstigatorAbility(Cast<UARTGameplayAbility_Order>(Ability));
}

UARTGameplayAbility_Order* UARTUseAbilityOrder::GetAbility(const UARTAbilitySystemComponent* AbilitySystem,
                                                  FGameplayTagContainer OrderTags) const
{
	TArray<FGameplayAbilitySpec*> SpecArray;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, false);
	if (SpecArray.Num() > 0)
	{
		FGameplayAbilitySpec* Spec = SpecArray[0];
		UGameplayAbility* CDOAbility = Spec->Ability;
		UGameplayAbility* InstancedAbility = Spec->GetPrimaryInstance();
		
		UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : CDOAbility;
		
		return Cast<UARTGameplayAbility_Order>(AbilitySource);
	}

	return nullptr;
}

UTexture2D* UARTUseAbilityOrder::GetIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                         int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return nullptr;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return nullptr;
	}

	return Ability->GetAbilityIcon();
}
