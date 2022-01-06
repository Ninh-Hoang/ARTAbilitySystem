// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrder.h"

UARTOrder::UARTOrder()
{
	OrderProcessPolicy = EARTOrderProcessPolicy::CAN_BE_CANCELED;
	//TagRequirements.SourceRequiredTags.AddTag(UARTGlobalTags::Status_Changing_IsAlive());
}

bool UARTOrder::CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                             FARTOrderErrorTags* OutErrorTags /*= nullptr*/) const
{
	return true;
}

bool UARTOrder::IsValidTarget(const AActor* OrderedActor, const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index,
                              FARTOrderErrorTags* OutErrorTags /*= nullptr*/) const
{
	return true;
}

void UARTOrder::CreateIndividualTargetLocations(const TArray<AActor*>& OrderedActors,
                                                const FARTOrderTargetData& TargetData,
                                                TArray<FVector2D>& OutTargetLocations) const
{
	for (int32 i = 0; i < OrderedActors.Num(); ++i)
	{
		OutTargetLocations.Add(TargetData.Location);
	}
}

void UARTOrder::IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                           const FGameplayTagContainer& OrderTags, int32 Index,
                           FARTOrderCallback Callback, const FVector& HomeLocation)
{
	check(0);
}

void UARTOrder::OrderCanceled(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                              const FGameplayTagContainer& OrderTags, int32 Index) const
{
}

EARTTargetType UARTOrder::GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                        int32 Index) const
{
	return EARTTargetType::NONE;
}

bool UARTOrder::IsCreatingIndividualTargetLocations(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                    int32 Index) const
{
	return false;
}

UTexture2D* UARTOrder::GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                     int32 Index) const
{
	return nullptr;
}

UTexture2D* UARTOrder::GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                      int32 Index) const
{
	return nullptr;
}

UTexture2D* UARTOrder::GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                      int32 Index) const
{
	return nullptr;
}

UTexture2D* UARTOrder::GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                       int32 Index) const
{
	return nullptr;
}

FText UARTOrder::GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return FText::FromString(GetClass()->GetName());
}

FText UARTOrder::GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return FText();
}

int32 UARTOrder::GetOrderButtonIndex() const
{
	return -1;
}

bool UARTOrder::HasFixedOrderButtonIndex() const
{
	return false;
}

FARTOrderPreviewData UARTOrder::GetOrderPreviewData(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                    int32 Index) const
{
	return FARTOrderPreviewData();
}

void UARTOrder::InitializePreviewActor(AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
	const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index)
{
}

void UARTOrder::GetTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                                   FARTOrderTagRequirements& OutTagRequirements) const
{
	OutTagRequirements = TagRequirements;
}

void UARTOrder::GetSuccessTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                          int32 Index,
                                          FARTOrderTagRequirements& OutTagRequirements) const
{
	OutTagRequirements = SuccessTagRequirements;
}

float UARTOrder::GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return 0;
}

bool UARTOrder::GetAcquisitionRadiusOverride(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                             int32 Index, float& OutAcquisitionRadius) const
{
	return false;
}

EARTOrderProcessPolicy UARTOrder::GetOrderProcessPolicy(const AActor* OrderedActor,
                                                        const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return OrderProcessPolicy;
}

TSoftClassPtr<UARTOrder> UARTOrder::GetFallbackOrder() const
{
	return FallbackOrder;
}

float UARTOrder::GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                const FGameplayTagContainer& OrderTags, int32 Index) const
{
	// TODO: Implement this function individually for each order type instead of here in the base class.
	if (!IsValid(OrderedActor))
	{
		return 0.0f;
	}

	float Distance = 0.0f;

	if (IsValid(TargetData.Actor))
	{
		// Subtract the half collision size of the target from the distance.
		// TODO: This is only relevant for melee units and should be ignored for ranged units.
		Distance = FVector::Dist2D(OrderedActor->GetActorLocation(), TargetData.Actor->GetActorLocation());

		//deduct by half the size incase of huge unit
		Distance -= TargetData.Actor->GetSimpleCollisionRadius();
	}

	else
	{
		Distance = FVector::Dist2D(OrderedActor->GetActorLocation(), FVector(TargetData.Location, 0.0f));
	}

	float AcquisitionRadius;
	if (!GetAcquisitionRadiusOverride(OrderedActor, OrderTags, Index, AcquisitionRadius))
	{
		// NOTE(np): In A Year Of Rain, units have a specific radius in which to automatically acquire targets.
		/*const UARTAttackComponent* AttackComponent = OrderedActor->FindComponentByClass<UARTAttackComponent>();
		if (AttackComponent == nullptr)
		{
		    return 0.0f;
		}

		AcquisitionRadius = AttackComponent->GetAcquisitionRadius();*/
		AcquisitionRadius = 100000.0f;
	}

	return 1.0f - Distance / AcquisitionRadius;
}

EARTOrderGroupExecutionType UARTOrder::GetGroupExecutionType(const AActor* OrderedActor,
                                                             const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return EARTOrderGroupExecutionType::ALL;
}

bool UARTOrder::IsHumanPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                       int32 Index) const
{
	return false;
}

bool UARTOrder::GetHumanPlayerAutoOrderInitialState(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                    int32 Index) const
{
	return false;
}

bool UARTOrder::IsAIPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                    int32 Index) const
{
	return false;
}

bool UARTOrder::AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const 
{
	return false;
}
