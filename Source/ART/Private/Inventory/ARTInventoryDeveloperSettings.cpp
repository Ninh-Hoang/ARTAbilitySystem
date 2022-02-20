// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ARTInventoryDeveloperSettings.h"

#include "Inventory/ARTItemStack.h"
#include "Inventory/Item/ARTItemStackWorldObject.h"

UARTInventoryDeveloperSettings::UARTInventoryDeveloperSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultItemStackClass = UARTItemStack::StaticClass();
	ItemStackWorldObjectClass = AARTItemStackWorldObject::StaticClass();
}
