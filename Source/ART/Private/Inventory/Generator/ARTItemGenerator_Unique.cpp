// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Generator/ARTItemGenerator_Unique.h"

#include "Inventory/ARTItemStack.h"
#include "Inventory/Item/ARTItemDefinition.h"

UARTItemGenerator_Unique::UARTItemGenerator_Unique(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UARTItemStack* UARTItemGenerator_Unique::GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context)
{
	if (!IsValid(ItemDefinition))
	{
		return nullptr;
	}

	UARTItemStack* ItemStack = CreateNewItemStack(ItemDefinition, ItemRarity);
	ItemStack->ItemName = ItemDisplayName; 

	for (UARTItemGenerator* SubItemGenerator : SubItemGenerators)
	{
		if (IsValid(SubItemGenerator))
		{
			UARTItemStack* SubItem = SubItemGenerator->GenerateItemStack(Context);

			//TODO: 
			//SubItem->OnGenerated();

			ItemStack->AddSubItemStack(SubItem);

			//TODO: 
			//SubItem->OnAddedToItemStack(ItemStack); //Or put this in AddSubItemStack?
		}
	}	

	return ItemStack;
}