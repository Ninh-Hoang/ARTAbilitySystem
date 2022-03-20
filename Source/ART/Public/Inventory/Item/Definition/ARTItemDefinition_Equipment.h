// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTItemDefinition_Container.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "ARTItemDefinition_Equipment.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ART_API UARTItemDefinition_Equipment : public UARTItemDefinition_Container
{
	GENERATED_BODY()
public:
	UARTItemDefinition_Equipment();
		
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Equipped Item")
	FARTItemDefinition_AbilityInfo EquippedItemAbilityInfo;
};
