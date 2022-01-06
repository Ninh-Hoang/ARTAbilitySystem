// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTDecorator_OrderTargetType.h"
#include "AIController.h"
#include "AI/Order/ARTOrderComponent.h"
#include "AI/Order/ARTOrderHelper.h"

UBTDecorator_OrderTargetType::UBTDecorator_OrderTargetType(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	NodeName = "Has Order Target Type";

	OrderTargetType = EARTTargetType::NONE;
}

bool UBTDecorator_OrderTargetType::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                              uint8* NodeMemory) const
{
	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	UARTOrderComponent* OrderComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UARTOrderComponent>();
	return OrderTargetType == UARTOrderHelper::GetTargetType(OrderComponent->GetCurrentOrderType(), Pawn,
	                               OrderComponent->GetCurrentOrderTagContainer(),
	                               OrderComponent->GetCurrentOrderTargetIndex());
}
