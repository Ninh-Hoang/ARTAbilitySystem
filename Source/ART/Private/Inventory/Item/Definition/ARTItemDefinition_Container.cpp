// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Item/Definition/ARTItemDefinition_Container.h"
#include "Inventory/Mod/ARTItemStack_SlotContainer.h"

UARTItemDefinition_Container::UARTItemDefinition_Container()
{
	DefaultItemStackClass = UARTItemStack_SlotContainer::StaticClass();
}
