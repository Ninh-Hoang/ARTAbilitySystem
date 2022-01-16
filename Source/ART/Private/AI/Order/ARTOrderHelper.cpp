// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderHelper.h"

#include "AbilitySystemComponent.h"
#include "VorbisAudioInfo.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"
#include "Ability/ARTGlobalTags.h"
#include "AI/Order/ARTAutoOrderComponent.h"
#include "AI/Order/ARTOrderComponent.h"
#include "AI/Order/ARTOrderWithBehavior.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

bool UARTOrderHelper::CanObeyOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                   const FGameplayTagContainer& OrderTags, int32 Index)
{
	return CanObeyOrder(OrderType, OrderedActor, OrderTags, Index, nullptr);
}

bool UARTOrderHelper::K2_CanObeyOrderWithErrorTags(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                   const FGameplayTagContainer& OrderTags, int32 Index,
                                                   FARTOrderErrorTags& OutErrorTags)
{
	// Clear the tag container. If this does not happen you may get old values when using this function from blueprint.
	OutErrorTags.MissingTags.Reset();
	OutErrorTags.BlockingTags.Reset();
	OutErrorTags.ErrorTags.Reset();

	return CanObeyOrder(OrderType, OrderedActor, OrderTags, Index, &OutErrorTags);
}

bool UARTOrderHelper::CanObeyOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                   const FGameplayTagContainer& OrderTags, int32 Index,
                                   FARTOrderErrorTags* OutErrorTags)
{
	if (OrderType == nullptr || !IsValid(OrderedActor))
	{
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	const UARTOrder* Order = OrderType->GetDefaultObject<UARTOrder>();
	const UAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UAbilitySystemComponent>();
	if (AbilitySystem != nullptr)
	{
		FARTOrderTagRequirements TagRequirements;
		Order->GetTagRequirements(OrderedActor, OrderTags, Index, TagRequirements);

		FGameplayTagContainer OrderedActorTags;
		AbilitySystem->GetOwnedGameplayTags(OrderedActorTags);

		if (OutErrorTags != nullptr)
		{
			if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirementsWithResult(
				OrderedActorTags, TagRequirements.SourceRequiredTags, TagRequirements.SourceBlockedTags,
				OutErrorTags->MissingTags, OutErrorTags->BlockingTags))
			{
				return false;
			}
		}
		else
		{
			if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(
				OrderedActorTags, TagRequirements.SourceRequiredTags, TagRequirements.SourceBlockedTags))
			{
				return false;
			}
		}
	}

	return Order->CanObeyOrder(OrderedActor, OrderTags, Index, OutErrorTags);
}

bool UARTOrderHelper::IsValidTarget(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                    const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags,
                                    int32 Index)
{
	return IsValidTarget(OrderType, OrderedActor, TargetData, OrderTags, Index, nullptr);
}

bool UARTOrderHelper::K2_IsValidTargetWithErrorTags(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                    const FARTOrderTargetData& TargetData,
                                                    const FGameplayTagContainer& OrderTags, int32 Index,
                                                    FARTOrderErrorTags& OutErrorTags)
{
	// Clear the tag container. If this does not happen you may get old values when using this function from blueprint.
	OutErrorTags.MissingTags.Reset();
	OutErrorTags.BlockingTags.Reset();
	OutErrorTags.ErrorTags.Reset();

	return IsValidTarget(OrderType, OrderedActor, TargetData, OrderTags, Index, &OutErrorTags);
}

bool UARTOrderHelper::IsValidTarget(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                    const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags,
                                    int32 Index,

                                    FARTOrderErrorTags* OutErrorTags)
{
	if (OrderType == nullptr)
	{
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	const UARTOrder* Order = OrderType->GetDefaultObject<UARTOrder>();

	EARTTargetType TargetType = Order->GetTargetType(OrderedActor, OrderTags, Index);
	if (TargetType == EARTTargetType::ACTOR)
	{
		if (!IsValid(TargetData.Actor))
		{
			if (OutErrorTags != nullptr)
			{
				OutErrorTags->ErrorTags.AddTag(UARTGlobalTags::AbilityActivationFailure_NoTarget());
			}
			return false;
		}

		FARTOrderTagRequirements TagRequirements;
		Order->GetTagRequirements(OrderedActor, OrderTags, Index, TagRequirements);

		if (OutErrorTags != nullptr)
		{
			if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirementsWithResult(
				TargetData.TargetTags, TagRequirements.TargetRequiredTags, TagRequirements.TargetBlockedTags,
				OutErrorTags->MissingTags, OutErrorTags->BlockingTags))
			{
				return false;
			}
		}

		else
		{
			if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(
				TargetData.TargetTags, TagRequirements.TargetRequiredTags, TagRequirements.TargetBlockedTags))
			{
				return false;
			}
		}
	}

	return Order->IsValidTarget(OrderedActor, TargetData, OrderTags, Index, OutErrorTags);
}

void UARTOrderHelper::CreateIndividualTargetLocations(TSoftClassPtr<UARTOrder> OrderType,
                                                      const TArray<AActor*>& OrderedActors,
                                                      const FARTOrderTargetData& TargetData,
                                                      TArray<FVector2D>& OutTargetLocations)
{
	if (OrderType == nullptr)
	{
		return;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	OrderType->GetDefaultObject<UARTOrder>()->CreateIndividualTargetLocations(OrderedActors, TargetData,
	                                                                          OutTargetLocations);

	if (OrderedActors.Num() != OutTargetLocations.Num())
	{
		UE_LOG(LogOrder, Error,
		       TEXT("The implementation of 'CreateIndividualTargetLocations' of class '%s' does return an amount of "
			       "locations that is different to the amount of actors."),
		       *OrderType->GetName());
	}
}

void UARTOrderHelper::IssueOrder(AActor* OrderedActor, const FARTOrderData& Order)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::IssueOrder: The specified actor is invalid."));
		return;
	}

	UARTOrderComponent* OrderComponent = OrderedActor->FindComponentByClass<UARTOrderComponent>();
	if (OrderComponent == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::IssueOrder: The specified actor has no order component."));
		return;
	}

	OrderComponent->IssueOrder(Order);
}

void UARTOrderHelper::ClearOrderQueue(AActor* OrderedActor)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::EnqueueOrder: The specified actor is invalid."));
		return;
	}

	UARTOrderComponent* OrderComponent = OrderedActor->FindComponentByClass<UARTOrderComponent>();
	if (OrderComponent == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::EnqueueOrder: The specified actor has no order component."));
		return;
	}

	OrderComponent->ClearOrderQueue();
}

void UARTOrderHelper::EnqueueOrder(AActor* OrderedActor, const FARTOrderData& Order)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::EnqueueOrder: The specified actor is invalid."));
		return;
	}

	UARTOrderComponent* OrderComponent = OrderedActor->FindComponentByClass<UARTOrderComponent>();
	if (OrderComponent == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::EnqueueOrder: The specified actor has no order component."));
		return;
	}

	OrderComponent->EnqueueOrder(Order);
}

void UARTOrderHelper::InsertOrderAfterCurrentOrder(AActor* OrderedActor, const FARTOrderData& Order)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::InsertOrderAfterCurrentOrder: The specified actor is invalid."));
		return;
	}

	UARTOrderComponent* OrderComponent = OrderedActor->FindComponentByClass<UARTOrderComponent>();
	if (OrderComponent == nullptr)
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderHelper::InsertOrderAfterCurrentOrder: The specified actor has no order component."));
		return;
	}

	OrderComponent->InsertOrderAfterCurrentOrder(Order);
}

EARTTargetType UARTOrderHelper::GetTargetType(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                              const FGameplayTagContainer& OrderTags, int32 Index)
{
	if (OrderType == nullptr)
	{
		return EARTTargetType::NONE;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetTargetType(OrderedActor, OrderTags, Index);
}

bool UARTOrderHelper::IsCreatingIndividualTargetLocations(TSoftClassPtr<UARTOrder> OrderType,
                                                          const AActor* OrderedActor,
                                                          const FGameplayTagContainer& OrderTags, int32 Index
)
{
	if (OrderType == nullptr)
	{
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->
	                  IsCreatingIndividualTargetLocations(OrderedActor, OrderTags, Index);
}

UBehaviorTree* UARTOrderHelper::GetBehaviorTree(TSoftClassPtr<UARTOrderWithBehavior> OrderType)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrderWithBehavior>()->GetBehaviorTree();
}

bool UARTOrderHelper::ShouldRestartBehaviourTree(TSoftClassPtr<UARTOrderWithBehavior> OrderType)
{
	if (OrderType == nullptr)
	{
		return true;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrderWithBehavior>()->ShouldRestartBehaviorTree();
}

FARTOrderTargetData UARTOrderHelper::CreateOrderTargetData(const AActor* OrderedActor, AActor* TargetActor,
                                                           const FVector2D& TargetLocation)
{
	FARTOrderTargetData TargetData;
	TargetData.Actor = TargetActor;
	TargetData.Location = TargetLocation;

	if (TargetActor == nullptr)
	{
		return TargetData;
	}

	FGameplayTagContainer SourceTags;
	FGameplayTagContainer TargetTags;
	UARTBlueprintFunctionLibrary::GetSourceAndTargetTags(OrderedActor, TargetActor, SourceTags, TargetTags);

	TargetData.TargetTags = TargetTags;
	return TargetData;
}

UTexture2D* UARTOrderHelper::GetNormalIcon(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor /*= nullptr*/,
                                           const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetNormalIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTOrderHelper::GetHoveredIcon(TSoftClassPtr<UARTOrder> OrderType,
                                            const AActor* OrderedActor /*= nullptr*/,
                                            const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetHoveredIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTOrderHelper::GetPressedIcon(TSoftClassPtr<UARTOrder> OrderType,
                                            const AActor* OrderedActor /*= nullptr*/,
                                            const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetPressedIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTOrderHelper::GetDisabledIcon(TSoftClassPtr<UARTOrder> OrderType,
                                             const AActor* OrderedActor /*= nullptr*/,
                                             const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetDisabledIcon(OrderedActor, OrderTags, Index);
}

FText UARTOrderHelper::GetName(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor /*= nullptr*/,
                               const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return FText::FromString(TEXT("UARTOrderHelper::GetName: Error: Parameter 'OrderType' was 'nullptr'."));
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetName(OrderedActor, OrderTags, Index);
}

FText UARTOrderHelper::GetDescription(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor /*= nullptr*/,
                                      const FGameplayTagContainer& OrderTags,
                                      int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return FText::FromString(TEXT("UARTOrderHelper::GetDescription: Error: Parameter 'OrderType' was 'nullptr'."));
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetDescription(OrderedActor, OrderTags, Index);
}

int32 UARTOrderHelper::GetOrderButtonIndex(TSoftClassPtr<UARTOrder> OrderType)
{
	if (OrderType == nullptr)
	{
		return -1;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetOrderButtonIndex();
}

bool UARTOrderHelper::HasFixedOrderButtonIndex(TSoftClassPtr<UARTOrder> OrderType)
{
	if (OrderType == nullptr)
	{
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->HasFixedOrderButtonIndex();
}

FARTOrderPreviewData UARTOrderHelper::GetOrderPreviewData(TSoftClassPtr<UARTOrder> OrderType,
                                                          const AActor* OrderedActor /*= nullptr*/,
                                                          const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr)
	{
		return FARTOrderPreviewData();
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetOrderPreviewData(OrderedActor, OrderTags, Index);
}

void UARTOrderHelper::InitializePreviewActor(TSoftClassPtr<UARTOrder> OrderType, AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
	const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index)
{
	if (OrderType == nullptr)
	{
		return;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}
	OrderType->GetDefaultObject<UARTOrder>()->InitializePreviewActor(PreviewActor, OrderedActor, TargetData, OrderTags, Index);
}

void UARTOrderHelper::GetOrderTagRequirements(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                              const FGameplayTagContainer& OrderTags, int32 Index,
                                              FARTOrderTagRequirements& OutTagRequirements)
{
	if (OrderType == nullptr)
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderHelper::GetOrderTagRequirements: Error: Parameter 'OrderType' was 'nullptr'."));
		return;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetTagRequirements(OrderedActor, OrderTags, Index,
	                                                                    OutTagRequirements);
}

void UARTOrderHelper::GetOrderSuccessTagRequirements(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                     const FGameplayTagContainer& OrderTags, int32 Index,
                                                     FARTOrderTagRequirements& OutTagRequirements)
{
	if (OrderType == nullptr)
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderHelper::GetOrderTagRequirements: Error: Parameter 'OrderType' was 'nullptr'."));
		return;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetSuccessTagRequirements(
		OrderedActor, OrderTags, Index, OutTagRequirements);
}

float UARTOrderHelper::GetOrderRequiredRange(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                             const FGameplayTagContainer& OrderTags, int32 Index)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::GetOrderRequiredRange: The specified actor is invalid."));
		return 0.0f;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetRequiredRange(OrderedActor, OrderTags, Index);
}

bool UARTOrderHelper::GetAcquisitionRadiusOverride(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                   const FGameplayTagContainer& OrderTags, int32 Index,
                                                   float& OutAcquisitionRadius)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderHelper::GetOrderAcquisitionRadiusOverride: The specified actor is invalid."));
		return 0.0f;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetAcquisitionRadiusOverride(OrderedActor, OrderTags, Index,
		OutAcquisitionRadius);
}

EARTOrderProcessPolicy UARTOrderHelper::GetOrderProcessPolicy(TSoftClassPtr<UARTOrder> OrderType,
                                                              const AActor* OrderedActor,
                                                              const FGameplayTagContainer& OrderTags,
                                                              int32 Index /*= -1*/)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::IsInstantOrder: The specified actor is invalid."));
		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	if (OrderType == nullptr)
	{
		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetOrderProcessPolicy(OrderedActor, OrderTags, Index);
}

TSoftClassPtr<UARTOrder> UARTOrderHelper::GetFallbackOrder(TSoftClassPtr<UARTOrder> OrderType)
{
	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetFallbackOrder();
}

float UARTOrderHelper::GetOrderTargetScore(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                           const FARTOrderTargetData& TargetData,
                                           const FGameplayTagContainer& OrderTags, int32 Index)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::GetOrderTargetScore: The specified actor is invalid."));
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
}

bool UARTOrderHelper::IsHumanPlayerAutoOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                             const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::IsHumanPlayerAutoOrder: The specified actor is invalid."));
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->IsHumanPlayerAutoOrder(OrderedActor, OrderTags, Index);
}

bool UARTOrderHelper::IsAIPlayerAutoOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                          const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::IsAIPlayerAutoOrder: The specified actor is invalid."));
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->IsAIPlayerAutoOrder(OrderedActor, OrderTags, Index);
}

bool UARTOrderHelper::GetHumanPlayerAutoOrderInitialState(TSoftClassPtr<UARTOrder> OrderType,
                                                          const AActor* OrderedActor,
                                                          const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error,
		       TEXT("UARTOrderHelper::GetHumanPlayerAutoOrderInitialState: The specified actor is invalid."));
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->
	                  GetHumanPlayerAutoOrderInitialState(OrderedActor, OrderTags, Index);
}

void UARTOrderHelper::SetHumanPlayerAutoOrderState(const AActor* OrderedActor, const FARTOrderTypeWithIndex& Order,
                                                   bool bEnable)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::SetHumanPlayerAutoOrderState: The specified actor is invalid."));
		return;
	}

	UARTAutoOrderComponent* AutoOrderComponent = OrderedActor->FindComponentByClass<UARTAutoOrderComponent>();

	if (!IsValid(AutoOrderComponent))
	{
		UE_LOG(
			LogOrder, Error,
			TEXT("UARTOrderHelper::SetHumanPlayerAutoOrderState: The specified actor has no ARTAutoOrderComponent."));
		return;
	}

	AutoOrderComponent->SetHumanPlayerAutoOrderState(Order, bEnable);
}

bool UARTOrderHelper::AreAutoOrdersAllowedDuringOrder(TSoftClassPtr<UARTOrder> OrderType,
																const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index)
{
	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	if(!OrderType.IsValid())
	{
		UE_LOG(LogOrder, Error, TEXT("UARTOrderHelper::AreAutoOrdersAllowedDuringOrderr: The specified actor is Has no current order to compare. Allow Auto Order."));
		return true;
	}

	return OrderType->GetDefaultObject<UARTOrder>()->AreAutoOrdersAllowedDuringOrder(OrderedActor, OrderTags, Index);
}

bool UARTOrderHelper::CanOrderBeConsideredAsSucceeded(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                      const FARTOrderTargetData& TargetData,
                                                      const FGameplayTagContainer& OrderTags, int32 Index /*= -1*/)
{
	if (OrderType == nullptr || !IsValid(OrderedActor))
	{
		return false;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	const UARTOrder* Order = OrderType->GetDefaultObject<UARTOrder>();
	const UAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UAbilitySystemComponent>();

	check(AbilitySystem != nullptr);

	FARTOrderTagRequirements TagRequirements;
	Order->GetSuccessTagRequirements(OrderedActor, OrderTags, Index, TagRequirements);

	FGameplayTagContainer OrderedActorTags;
	AbilitySystem->GetOwnedGameplayTags(OrderedActorTags);

	if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(OrderedActorTags, TagRequirements.SourceRequiredTags,
	                                                              TagRequirements.SourceBlockedTags))
	{
		return false;
	}

	if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(TargetData.TargetTags,
	                                                              TagRequirements.TargetRequiredTags,
	                                                              TagRequirements.TargetBlockedTags))
	{
		return false;
	}

	return true;
}

EARTOrderGroupExecutionType UARTOrderHelper::GetOrderGroupExecutionType(TSoftClassPtr<UARTOrder> OrderType,
                                                                        const AActor* OrderedActor,
                                                                        const FGameplayTagContainer& OrderTags,
                                                                        int32 Index /*= -1*/
)
{
	if (OrderType == nullptr)
	{
		return EARTOrderGroupExecutionType::ALL;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	return OrderType->GetDefaultObject<UARTOrder>()->GetGroupExecutionType(OrderedActor, OrderTags, Index);
}

FARTOrderErrorTags UARTOrderHelper::CheckOrder(AActor* OrderedActor, const FARTOrderData& Order)
{
	// TODO: Detailed check with callback with parameters that describe why the order could not be issued.

	FARTOrderErrorTags OrderErrorTags;

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
		return OrderErrorTags;
	}

	if (!UARTOrderHelper::CanObeyOrder(OrderType.Get(), OrderedActor, Order.OrderTags, Order.Index, &OrderErrorTags))
	{
		return OrderErrorTags;
	}

	FARTOrderTargetData TargetData = UARTOrderHelper::CreateOrderTargetData(OrderedActor, Order.Target, Order.Location);
	if (!UARTOrderHelper::IsValidTarget(OrderType.Get(), OrderedActor, TargetData, Order.OrderTags, Order.Index,
	                                    &OrderErrorTags))
	{
		return OrderErrorTags;
	}

	return OrderErrorTags;
}

bool UARTOrderHelper::IsEnemyInAcquisitionRadius(const AActor* OrderedActor, float AcquisitionRadius)
{
	if (!IsValid(OrderedActor))
	{
		return false;
	}

	TArray<AActor*> ActorsInRange;
	FindActors(OrderedActor->GetWorld(), AcquisitionRadius, OrderedActor->GetActorLocation(), ActorsInRange);

	for (AActor* Actor : ActorsInRange)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		// Check the target tags.
		if (UARTBlueprintFunctionLibrary::GetTeamAttitudeTags(OrderedActor, Actor)
			.HasTag(UARTGlobalTags::Behaviour_Hostile()))
		{
			return true;
		}
	}

	return false;
}

AActor* UARTOrderHelper::FindTargetForOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                            const FGameplayTagContainer& OrderTags, int32 Index,

                                            float AcquisitionRadius, float& OutScore)
{
	if (!IsValid(OrderedActor))
	{
		return nullptr;
	}
	
	return FindTargetForOrderInChaseDistance(OrderType, OrderedActor, OrderTags, Index, AcquisitionRadius,
	                                         AcquisitionRadius,
	                                         OrderedActor->GetActorLocation(), OutScore);
}

AActor* UARTOrderHelper::FindTargetForOrderInChaseDistance(TSoftClassPtr<UARTOrder> OrderType,
                                                           const AActor* OrderedActor,
                                                           const FGameplayTagContainer& OrderTags, int32 Index,
                                                           float AcquisitionRadius, float ChaseDistance,
                                                           const FVector& OrderedActorHomeLocation, float& OutScore)
{
	if (!IsValid(OrderedActor))
	{
		return nullptr;
	}

	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	const UARTOrder* Order = OrderType->GetDefaultObject<UARTOrder>();

	// Only target types with a real target location are relevant.
	EARTTargetType TargetType = Order->GetTargetType(OrderedActor, OrderTags, Index);
	if (TargetType == EARTTargetType::NONE || TargetType == EARTTargetType::PASSIVE)
	{
		return nullptr;
	}

	// Find all targets in acquisition radius.
	TArray<AActor*> ActorsInRange;
	FindActorsInChaseDistance(OrderedActor->GetWorld(), AcquisitionRadius, ChaseDistance,
	                          OrderedActor->GetActorLocation(), OrderedActorHomeLocation, ActorsInRange);

	if (ActorsInRange.Num() == 0)
	{
		return nullptr;
	}

	return FindBestScoredTargetForOrder(OrderType, OrderedActor, ActorsInRange, OrderTags, Index, OutScore);
}

void UARTOrderHelper::FindActors(UObject* WorldContextObject, float AcquisitionRadius,
                                 const FVector& OrderedActorLocation, TArray<AActor*>& OutActorsInRange)
{
	FindActorsInChaseDistance(WorldContextObject, AcquisitionRadius, AcquisitionRadius, OrderedActorLocation,
	                          OrderedActorLocation, OutActorsInRange);
}

void UARTOrderHelper::FindActorsInChaseDistance(UObject* WorldContextObject, float AcquisitionRadius,
                                                float ChaseDistance, const FVector& OrderedActorLocation,
                                                const FVector& OrderedActorHomeLocation,
                                                TArray<AActor*>& OutActorsInRange)
{
	TArray<AActor*> ActorsInRange;
	TArray<AActor*> ActorsToIgnore;

	// NOTE(np): In A Year Of Rain, we're storing detection channels for units in the game instance.
	// Find all targets in acquisition radius.
	//UARTGameInstance* GameInstance = UARTUtilities::GetARTGameInstance(WorldContextObject);
	TArray<TEnumAsByte<EObjectTypeQuery>> QueryChannel;
	QueryChannel.Add(ObjectTypeQuery3);
	UKismetSystemLibrary::CapsuleOverlapActors(WorldContextObject, OrderedActorLocation, AcquisitionRadius, 10000.0f,
	                                           QueryChannel, APawn::StaticClass(),
	                                           ActorsToIgnore, ActorsInRange);

	if (ActorsInRange.Num() == 0)
	{
		return;
	}

	// Filter the array for valid targets.
	for (AActor* Actor : ActorsInRange)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		FVector ActorLocation = Actor->GetActorLocation();

		// If the target is too far away from our home location it is invalid.
		float DistanceToHomeLocation = FVector::Dist2D(ActorLocation, OrderedActorHomeLocation);
		if (DistanceToHomeLocation > ChaseDistance)
		{
			continue;
		}

		OutActorsInRange.Add(Actor);
	}
}

AActor* UARTOrderHelper::FindBestScoredTargetForOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                      const TArray<AActor*> Targets,
                                                      const FGameplayTagContainer& OrderTags, int32 Index,
                                                      float& OutScore)
{
	if (!IsValid(OrderedActor))
	{
		return nullptr;
	}

	if (OrderType == nullptr)
	{
		return nullptr;
	}

	if (!OrderType.IsValid())
	{
		OrderType.LoadSynchronous();
	}

	const UARTOrder* Order = OrderType->GetDefaultObject<UARTOrder>();

	// Filter the array for valid targets.
	FARTOrderTagRequirements TagRequirements;
	Order->GetTagRequirements(OrderedActor, OrderTags, Index, TagRequirements);
	TArray<TTuple<AActor*, float>> ActorsWithScore;
	for (AActor* Actor : Targets)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		// Check the target tags.
		FGameplayTagContainer TargetTags;
		UARTBlueprintFunctionLibrary::GetTags(Actor, TargetTags);
		TargetTags.AppendTags(UARTBlueprintFunctionLibrary::GetTeamAttitudeTags(OrderedActor, Actor));
		if (!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(TargetTags, TagRequirements.TargetRequiredTags,
		                                                              TagRequirements.TargetBlockedTags))
		{
			continue;
		}

		// Apply the order specific valid target check.
		FARTOrderTargetData OrderTargetData(Actor, FVector2D(Actor->GetActorLocation()), TargetTags);
		if (!Order->IsValidTarget(OrderedActor, OrderTargetData, OrderTags, Index))
		{
			continue;
		}

		// This actor is valid. Store it and its score in the array.
		ActorsWithScore.Emplace(Actor, Order->GetTargetScore(OrderedActor, OrderTargetData, OrderTags, Index));
	}

	// Find the best best target out of all potential targets using the score.
	TTuple<AActor*, float> HighestScoredActor;
	for (TTuple<AActor*, float> ActorWithScore : ActorsWithScore)
	{
		if (HighestScoredActor.Get<1>() < ActorWithScore.Get<1>())
		{
			HighestScoredActor = ActorWithScore;
		}
	}

	OutScore = HighestScoredActor.Get<1>();
	return HighestScoredActor.Get<0>();
}

AActor* UARTOrderHelper::FindMostSuitableActorToObeyTheOrder(TSoftClassPtr<UARTOrder> OrderType,
                                                             const TArray<AActor*> OrderedActors,
                                                             const FARTOrderTargetData TargetData,
                                                             const FGameplayTagContainer& OrderTags, int32 OrderIndex)
{
	TArray<TTuple<AActor*, float>> ActorsWithScore;
	for (AActor* Actor : OrderedActors)
	{
		if (UARTOrderHelper::CanObeyOrder(OrderType, Actor, OrderTags, OrderIndex))
		{
			// This actor is valid. Store it and its score in the array.
			ActorsWithScore.Emplace(Actor, GetOrderTargetScore(OrderType, Actor, TargetData, OrderTags, OrderIndex));
		}
	}

	if (ActorsWithScore.Num() == 0)
	{
		return nullptr;
	}

	// Find the best best actor out of all potential selected actors using the score.
	TTuple<AActor*, float> HighestScoredActor = ActorsWithScore[0];

	for (TTuple<AActor*, float> ActorWithScore : ActorsWithScore)
	{
		if (HighestScoredActor.Get<1>() < ActorWithScore.Get<1>())
		{
			HighestScoredActor = ActorWithScore;
		}
	}

	return HighestScoredActor.Get<0>();
}

FARTOrderTargetData UARTOrderHelper::FindOrderTargetDataFromAbility(const AActor* AbilityActor,
	const FGameplayTagContainer& AbilityTags)
{
	UAbilitySystemComponent* ASC = UARTBlueprintFunctionLibrary::GetAbilitySystemComponent(const_cast<AActor*>(AbilityActor));
	if(ASC)
	{
		TArray<FGameplayAbilitySpec*> SpecArray;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags, SpecArray);

		if (SpecArray.Num() > 0)
		{
			UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(SpecArray[0]->Ability);
			if(Ability) return Ability->GetOrderTargetData();
		}
	}

	return FARTOrderTargetData();
}
