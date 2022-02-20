// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Input/ARTItemInputBinder.h"

UARTItemInputBinder::UARTItemInputBinder() : Super()
{
}

int32 UARTItemInputBinder::GetInputBinding_Implementation(UAbilitySystemComponent* ASC,
	TSubclassOf<UGameplayAbility> AbilityToGrant)
{
	return INDEX_NONE;
}

