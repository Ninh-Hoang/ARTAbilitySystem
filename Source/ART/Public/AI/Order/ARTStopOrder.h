// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacterAIOrder.h"
#include "ARTStopOrder.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintType, Abstract, Blueprintable)
class ART_API UARTStopOrder : public UARTCharacterAIOrder
{
	GENERATED_BODY()
	public:
	UARTStopOrder();

	//~ Begin URTSOrder Interface
	virtual bool AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const override; 
	//~ End URTSOrder Interface
};
