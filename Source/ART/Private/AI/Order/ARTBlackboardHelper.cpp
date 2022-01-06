// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTBlackboardHelper.h"

#include "AI/Order/ARTOrderData.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName UARTBlackboardHelper::BLACKBOARD_KEY_ORDER_TYPE = TEXT("Order_OrderType");
const FName UARTBlackboardHelper::BLACKBOARD_KEY_LOCATION = TEXT("Order_Location");
const FName UARTBlackboardHelper::BLACKBOARD_KEY_TARGET = TEXT("Order_Target");
const FName UARTBlackboardHelper::BLACKBOARD_KEY_INDEX = TEXT("Order_Index");
const FName UARTBlackboardHelper::BLACKBOARD_KEY_RANGE = TEXT("Order_Range");
const FName UARTBlackboardHelper::BLACKBOARD_KEY_HOME_LOCATION = TEXT("Order_HomeLocation");

TSubclassOf<UARTOrder> UARTBlackboardHelper::GetBlackboardOrderType(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? BlackboardComp->GetValueAsClass(BLACKBOARD_KEY_ORDER_TYPE) : nullptr;
}

FVector UARTBlackboardHelper::GetBlackboardOrderLocation(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? BlackboardComp->GetValueAsVector(BLACKBOARD_KEY_LOCATION) : FVector::ZeroVector;
}

AActor* UARTBlackboardHelper::GetBlackboardOrderTarget(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(BLACKBOARD_KEY_TARGET)) : nullptr;
}

int32 UARTBlackboardHelper::GetBlackboardOrderIndex(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? BlackboardComp->GetValueAsInt(BLACKBOARD_KEY_INDEX) : 0;
}

float UARTBlackboardHelper::GetBlackboardOrderRange(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? BlackboardComp->GetValueAsFloat(BLACKBOARD_KEY_RANGE) : 0.0f;
}

FVector UARTBlackboardHelper::GetBlackboardOrderHomeLocation(UBTNode* NodeOwner)
{
    UBlackboardComponent* BlackboardComp = UBTFunctionLibrary::GetOwnersBlackboard(NodeOwner);
    return BlackboardComp ? BlackboardComp->GetValueAsVector(BLACKBOARD_KEY_HOME_LOCATION) : FVector::ZeroVector;
}
