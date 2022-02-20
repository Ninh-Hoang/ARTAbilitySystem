// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ARTInventoryAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"

UARTInventoryAttributeSet::UARTInventoryAttributeSet(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventorySize(1)
{
	
}

void UARTInventoryAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
															
	DOREPLIFETIME_CONDITION_NOTIFY(UARTInventoryAttributeSet, InventorySize, COND_None, REPNOTIFY_Always);
}

void UARTInventoryAttributeSet::OnRep_InventorySize(FGameplayAttributeData OldInventorySize)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UARTInventoryAttributeSet, InventorySize, OldInventorySize);
}

FGameplayAttribute UARTInventoryAttributeSet::InventorySizeAttribute()
{
	static FProperty* Property = FindFProperty<FProperty>(UARTInventoryAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UARTInventoryAttributeSet, InventorySize));
	return FGameplayAttribute(Property);
}	