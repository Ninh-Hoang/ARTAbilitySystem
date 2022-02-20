// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventorySet.generated.h"

/**
 * 
 */

UCLASS()
class ART_API UInventorySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	void InitInventory(class UARTInventoryComponent* InventoryComponent) const;
};
