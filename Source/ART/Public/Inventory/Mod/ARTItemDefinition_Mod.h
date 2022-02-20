// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "ARTItemDefinition_Mod.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ART_API UARTItemDefinition_Mod : public UARTItemDefinition
{
	GENERATED_BODY()
public:
	UARTItemDefinition_Mod();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perk Settings")
	TArray<TSubclassOf<class UGameplayEffect>> GameplayEffects;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Perk Settings")
	TArray<UAttributeSet*> AttributeSets;
};
