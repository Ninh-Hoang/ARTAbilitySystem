// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "ARTItemDefinition_Container.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ART_API UARTItemDefinition_Container : public UARTItemDefinition
{
	GENERATED_BODY()
public:
	UARTItemDefinition_Container();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Internal Container")
	TArray< FARTItemSlotDefinition> CustomInventorySlots;
};
