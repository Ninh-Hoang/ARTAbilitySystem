// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Generator/ARTItemGenerator.h"

#include "Inventory/ARTItemStack.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/Item/Definition/ARTItemDefinition_Container.h"
#include "Inventory/Mod/ARTItemStack_SlotContainer.h"

UARTItemGenerator::UARTItemGenerator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FPrimaryAssetId UARTItemGenerator::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UClass* SeARThNativeClass = GetClass();

		while (SeARThNativeClass && !SeARThNativeClass->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic))
		{
			SeARThNativeClass = SeARThNativeClass->GetSuperClass();
		}

		if (SeARThNativeClass && SeARThNativeClass != GetClass())
		{
			// If blueprint, return native class and asset name
			return FPrimaryAssetId(TEXT("ARTItemGenerator"), FPackageName::GetShortFName(GetOutermost()->GetFName()));
		}

		// Native CDO, return nothing
		return FPrimaryAssetId();
	}

	return  GetClass()->GetDefaultObject()->GetPrimaryAssetId();
}

UARTItemStack* UARTItemGenerator::GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context)
{
	return nullptr;
}

UARTItemStack* UARTItemGenerator::CreateNewItemStack(UARTItemDefinition* ItemDefinition, UARTItemRarity* ItemRarity)
{	
	TSubclassOf<UARTItemStack> ISC = ItemStackClass;

	if (IsValid(ISC))
	{
		//Check to see if our ItemStack Class is a child of what the item def wants.  If not, use the item def's class
		if (IsValid(ItemDefinition->DefaultItemStackClass) && !ISC->IsChildOf(ItemDefinition->DefaultItemStackClass))
		{
			ISC = ItemDefinition->DefaultItemStackClass;
		}
	}

	if (!IsValid(ISC))
	{
		//If we don't have a valid item stack class, use the one in the developer settings.
		//TODO:Inventory developer setting
		//ISC = GetDefault<UARTInventoryDeveloperSettings>()->DefaultItemStackClass;
		ISC = ItemDefinition->DefaultItemStackClass;
		if (!IsValid(ISC))
		{
			//If we still don't have a valid one, use the default item stack class.  That's good enough
			ISC = UARTItemStack::StaticClass();
		}
	}
	UARTItemStack* NewItemStack = NewObject<UARTItemStack>(GetTransientPackage(), ISC);
	NewItemStack->ItemDefinition = ItemDefinition;
	NewItemStack->Rarity = ItemRarity;
	PostCreateNewItemStack(NewItemStack);
	
	return NewItemStack;
}

void UARTItemGenerator::PostCreateNewItemStack(UARTItemStack* ItemStack)
{
	if(!ItemStack) return;
	if(UARTItemStack_SlotContainer* ContainerStack = Cast<UARTItemStack_SlotContainer>(ItemStack))
	{
		if(UARTItemDefinition_Container* ContainerDefinition = Cast<UARTItemDefinition_Container>(ItemStack->GetItemDefinition()))
		{
			ContainerStack->InitializeContainer(ContainerDefinition->CustomInventorySlots);
		}
	}
}

UARTItemGenerator_Static::UARTItemGenerator_Static(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UARTItemStack* UARTItemGenerator_Static::GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context)
{
	if (IsValid(StaticGenerator))
	{
		return StaticGenerator->GetDefaultObject<UARTItemGenerator>()->GenerateItemStack(Context);
	}

	return nullptr;
}
