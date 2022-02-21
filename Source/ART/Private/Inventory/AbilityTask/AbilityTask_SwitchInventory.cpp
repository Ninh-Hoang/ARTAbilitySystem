// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/AbilityTask/AbilityTask_SwitchInventory.h"

UAbilityTask_SwitchInventory::UAbilityTask_SwitchInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAbilityTask_SwitchInventory* UAbilityTask_SwitchInventory::SwitchInventorySlots(UGameplayAbility* OwningAbility, const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot)
{
	UAbilityTask_SwitchInventory* Task = NewAbilityTask<UAbilityTask_SwitchInventory>(OwningAbility);
	Task->FromSlot = FromSlot;
	Task->ToSlot = ToSlot;

	return Task;
} 	   

FGameplayAbilityTargetDataHandle UAbilityTask_SwitchInventory::GenerateTargetHandle()
{
	FGameplayAbilityTargetData_ItemSwitch* ItemSwitchData = new FGameplayAbilityTargetData_ItemSwitch();
	ItemSwitchData->FromSlot = FromSlot;
	ItemSwitchData->ToSlot = ToSlot;

	return FGameplayAbilityTargetDataHandle(ItemSwitchData);
}

void UAbilityTask_SwitchInventory::HandleTargetData(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData_ItemSwitch* SwitchData = static_cast<const FGameplayAbilityTargetData_ItemSwitch*>(Data.Get(0));
	if (SwitchData != nullptr)
	{
		OnSlotsReceived.Broadcast(SwitchData->FromSlot, SwitchData->ToSlot);
	}
	else
	{
		OnSlotsCancelled.Broadcast();
	}
	EndTask();
}

void UAbilityTask_SwitchInventory::HandleCancelled()
{
	OnSlotsCancelled.Broadcast();
	EndTask();
}

bool FGameplayAbilityTargetData_ItemSwitch::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	FromSlot.NetSerialize(Ar, Map, bOutSuccess);
	ToSlot.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}