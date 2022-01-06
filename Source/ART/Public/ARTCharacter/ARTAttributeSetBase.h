// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "ARTAttributeSetBase.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UARTAttributeSetBase();

protected:
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
                                     const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
                                     const FGameplayAttribute& AffectedAttributeProperty);
};
