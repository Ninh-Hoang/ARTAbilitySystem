// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GameplayTagBase.generated.h"

/**
 * 
 */
class UBehaviorTree;

UCLASS(Abstract)
class ART_API UBTTask_GameplayTagBase : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

	/** get name of selected blackboard key */
	FText GetSeletectedGameplayTag() const;
	
protected:
	
	/** gameplaytag selector */
	UPROPERTY(EditAnywhere, Category=GameplayTag)
	FGameplayTagContainer GameplayTagContainer;
	
};

//////////////////////////////////////////////////////////////////////////
// Inlines
FORCEINLINE FText UBTTask_GameplayTagBase::GetSeletectedGameplayTag() const
{
	return GameplayTagContainer.ToMatchingText(EGameplayContainerMatchType::All, false);
}
