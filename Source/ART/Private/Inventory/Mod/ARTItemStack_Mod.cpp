// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Mod/ARTItemStack_Mod.h"
#include "Net/UnrealNetwork.h"

UARTItemStack_Mod::UARTItemStack_Mod(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UARTItemStack_Mod::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARTItemStack_Mod, Modifiers);
}

bool UARTItemStack_Mod::CanAttachTo_Implementation(UARTItemStack* OtherStack)
{
	//Mods can't contain Mods
	if (OtherStack->IsA(UARTItemStack_Mod::StaticClass()))
	{
		return false;
	}

	return true;
}