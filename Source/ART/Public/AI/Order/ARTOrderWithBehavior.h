// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AI/Order/ARTOrderWithDisplay.h"
#include "ARTOrderWithBehavior.generated.h"

/**
 * 
 */

class AActor;
class UBehaviorTree;


UCLASS(BlueprintType, Abstract, Blueprintable)
class ART_API UARTOrderWithBehavior : public UARTOrderWithDisplay
{
	GENERATED_BODY()
	
public:
	UARTOrderWithBehavior();

	/** Gets the behavior tree that should run in order to obey this order. */
	UBehaviorTree* GetBehaviorTree() const;

	/** Whether to restart the behavior tree whenever a new order of this type is issued. */
	bool ShouldRestartBehaviorTree() const;

	/** Whether to loop the behavior tree. */
	bool ShouldLoopBehaviourTree() const;

	//~ Begin URTSOrder Interface
	virtual void IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index,
							FARTOrderCallback Callback, const FVector& HomeLocation) override;
	virtual bool GetAcquisitionRadiusOverride(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
											float& OutAcquisitionRadius) const override;
	//~ End URTSOrder Interface

private:
	/** The behavior tree that should run in order to obey this order. */
	UPROPERTY(Category = "Behavior", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UBehaviorTree* BehaviorTree;

	/** Whether to restart the behavior tree whenever a new order of this type is issued. */
	UPROPERTY(Category = "Behavior", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bShouldRestartBehaviorTree;

	/** Whether to loop the behavior tree. */
	UPROPERTY(Category = "Behavior", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bShouldLoopBehaviourTree;
	
	/** The specific acquisition radius for this order. */
	UPROPERTY(Category = "Behavior", EditDefaultsOnly, BlueprintReadOnly,
			meta = (AllowPrivateAccess = true, EditCondition = bIsAcquisitionRadiusOverridden))
	float AcquisitionRadiusOverride;

	/** Whether this order uses a specific acquisition radius. */
	UPROPERTY(Category = "Behavior", EditDefaultsOnly, BlueprintReadOnly,
			meta = (AllowPrivateAccess = true, InlineEditConditionToggle = true))
	bool bIsAcquisitionRadiusOverridden;
};
