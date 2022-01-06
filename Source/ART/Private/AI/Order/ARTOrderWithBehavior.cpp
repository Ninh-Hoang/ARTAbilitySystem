// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderWithBehavior.h"
#include "AI/Order/ARTOrder.h"
#include "ART/ART.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "ARTCharacter/AI/ARTAIController.h"


UARTOrderWithBehavior::UARTOrderWithBehavior()
{
	bShouldRestartBehaviorTree = true;
	AcquisitionRadiusOverride = 0.0f;
	bIsAcquisitionRadiusOverridden = false;
}

UBehaviorTree* UARTOrderWithBehavior::GetBehaviorTree() const
{
	return BehaviorTree;
}

bool UARTOrderWithBehavior::ShouldRestartBehaviorTree() const
{
	return bShouldRestartBehaviorTree;
}

void UARTOrderWithBehavior::IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                       const FGameplayTagContainer& OrderTags, int32 Index,
                                       FARTOrderCallback Callback, const FVector& HomeLocation)
{
	if (!IsValid(OrderedActor))
	{
		UE_LOG(LogOrder, Error, TEXT("Ordered actor is invalid."));
		Callback.Broadcast(EARTOrderResult::FAILED);
		return;
	}

	APawn* Pawn = Cast<APawn>(OrderedActor);
	if (Pawn == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("The specified actor '%s' is not a pawn."), *OrderedActor->GetName());
		Callback.Broadcast(EARTOrderResult::FAILED);
		return;
	}

	AARTAIController* Controller = Cast<AARTAIController>(Pawn->GetController());
	if (Controller == nullptr)
	{
		UE_LOG(
			LogOrder, Error,
			TEXT(
				"The specified pawn '%s' does not have the required 'ARTCharacterAIController' to receive ART orders."),
			*OrderedActor->GetName());
		Callback.Broadcast(EARTOrderResult::FAILED);
		return;
	}

	AActor* TargetActor = TargetData.Actor;
	const FVector2D TargetLocation = TargetData.Location;
	FARTOrderData Order(GetClass(), Index, OrderTags, TargetActor, TargetLocation);
	EARTTargetType TargetType = GetTargetType(OrderedActor, OrderTags, Index);

	Order.bUseLocation = TargetType == EARTTargetType::LOCATION || TargetType == EARTTargetType::DIRECTION;
	Controller->IssueOrder(Order, Callback, HomeLocation);
}

bool UARTOrderWithBehavior::GetAcquisitionRadiusOverride(const AActor* OrderedActor,
                                                         const FGameplayTagContainer& OrderTags, int32 Index,
                                                         float& OutAcquisitionRadius) const
{
	OutAcquisitionRadius = AcquisitionRadiusOverride;
	return bIsAcquisitionRadiusOverridden;
}
