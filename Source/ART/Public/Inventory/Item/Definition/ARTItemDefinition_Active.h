// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item/Definition/ARTItemDefinition_Equipment.h"
#include "ARTItemDefinition_Active.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ART_API UARTItemDefinition_Active : public UARTItemDefinition_Equipment
{
	GENERATED_BODY()
	
public:
	UARTItemDefinition_Active();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Display")
	USkeletalMesh* HeldItemMesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Active Item")
	FARTItemDefinition_AbilityInfo ActiveItemAbilityInfo;
};
