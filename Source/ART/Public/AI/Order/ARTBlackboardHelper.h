// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ARTOrder.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BTNode.h"
#include "ARTBlackboardHelper.generated.h"

/**
 * 
 */
/** Helper function for the behavior trees. */
UCLASS(meta = (RestrictedToClasses = "BTNode"))
class ART_API UARTBlackboardHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static TSubclassOf<UARTOrder> GetBlackboardOrderType(UBTNode* NodeOwner);

	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static FVector GetBlackboardOrderLocation(UBTNode* NodeOwner);

	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static AActor* GetBlackboardOrderTarget(UBTNode* NodeOwner);

	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static int32 GetBlackboardOrderIndex(UBTNode* NodeOwner);

	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static float GetBlackboardOrderRange(UBTNode* NodeOwner);

	UFUNCTION(BlueprintPure, Category = "ART|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static FVector GetBlackboardOrderHomeLocation(UBTNode* NodeOwner);

	static const FName BLACKBOARD_KEY_ORDER_TYPE;
	static const FName BLACKBOARD_KEY_LOCATION;
	static const FName BLACKBOARD_KEY_TARGET;
	static const FName BLACKBOARD_KEY_INDEX;
	static const FName BLACKBOARD_KEY_RANGE;
	static const FName BLACKBOARD_KEY_HOME_LOCATION;
};
