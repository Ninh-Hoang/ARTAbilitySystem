// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/InventorySet.h"
#include <Item/InventoryComponent.h>
#include "Item/Item.h"

void UInventorySet::InitInventory(class UInventoryComponent* InventoryComponent) const
{
	if (!InventoryComponent)
	{
		return;
	}

	for (const FItemApplicationInfo& Item : StartInventory)
	{
		if (Item.ItemClass)
		{
			InventoryComponent->TryAddItemFromClass(Item.ItemClass, Item.Quantity);
		}
	}
}
