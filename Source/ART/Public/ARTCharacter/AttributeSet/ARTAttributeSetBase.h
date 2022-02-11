// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARTAttributeSetBase.generated.h"

/**
 * 
 */
// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ART_API UARTAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UARTAttributeSetBase();

protected:
	virtual void AdjustAttributeForMaxChange(float OldMaxValue, float NewMaxValue,
                                     const FGameplayAttribute& AffectedAttributeProperty);
};
