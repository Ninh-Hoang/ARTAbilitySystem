// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AI/Order/ARTOrderTargetType.h"


#include "BTDecorator_OrderTargetType.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UBTDecorator_OrderTargetType : public UBTDecorator
{
	GENERATED_UCLASS_BODY()

protected:
	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	EARTTargetType OrderTargetType;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
