// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "UObject/Interface.h"
#include "InventoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInventoryInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Returns the map of items to data */
	virtual const FItemDataContainer& GetInventoryData() const = 0;

	/** Returns the map of slots to items */
	//virtual const TMap<FRPGItemSlot, URPGItem*>& GetSlottedItemMap() const = 0;

	/** Gets the delegate for inventory item changes */
	//virtual FOnInventoryItemChangedNative& GetInventoryItemChangedDelegate() = 0;

	/** Gets the delegate for inventory slot changes */
	//virtual FOnSlottedItemChangedNative& GetSlottedItemChangedDelegate() = 0;

	/** Gets the delegate for when the inventory loads */
	//virtual FOnInventoryLoadedNative& GetInventoryLoadedDelegate() = 0;
};
