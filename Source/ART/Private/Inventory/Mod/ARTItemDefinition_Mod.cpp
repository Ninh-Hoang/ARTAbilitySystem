// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Mod/ARTItemDefinition_Mod.h"
#include "Inventory/Mod/ARTItemStack_Mod.h"

UARTItemDefinition_Mod::UARTItemDefinition_Mod()
	: Super()
{
	DefaultItemStackClass = UARTItemStack_Mod::StaticClass();
}
