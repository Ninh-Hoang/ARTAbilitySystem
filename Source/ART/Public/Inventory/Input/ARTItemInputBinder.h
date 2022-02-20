// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ARTItemInputBinder.generated.h"

/**
 * 
 */

class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class ART_API UARTItemInputBinder : public UObject
{
	GENERATED_BODY()
	
public:
	UARTItemInputBinder();

	UFUNCTION(BlueprintNativeEvent)
	int32 GetInputBinding(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayAbility> AbilityToGrant);
	virtual int32 GetInputBinding_Implementation(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayAbility> AbilityToGrant);
};
