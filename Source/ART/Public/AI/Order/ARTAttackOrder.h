// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Order/ARTUseAbilityOrder.h"
#include "ARTAttackOrder.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintType, Abstract, Blueprintable)
class ART_API UARTAttackOrder : public UARTUseAbilityOrder
{
	GENERATED_BODY()
	
public:
	UARTAttackOrder();

	//~ Begin UARTOrder Interface

	virtual bool CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index, FARTOrderErrorTags* OutErrorTags) const override;
	
	virtual void CreateIndividualTargetLocations(const TArray<AActor*>& OrderedActors,
												const FARTOrderTargetData& TargetData,
												TArray<FVector2D>& OutTargetLocations) const override;
	
	virtual float GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;

	virtual float GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index) const override;

	virtual bool AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const override;
	//~ End UARTOrder Interface
};
