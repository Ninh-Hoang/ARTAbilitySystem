// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/AbilityTask/AbilityTask_DropItem.h"

bool FGameplayAbilityTargetData_DropItem::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	FromSlot.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}



UAbilityTask_DropItem::UAbilityTask_DropItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAbilityTask_DropItem* UAbilityTask_DropItem::DropItemFromInventory(UGameplayAbility* OwningAbility, const FARTItemSlotRef& FromSlot)
{
	UAbilityTask_DropItem* Task = NewAbilityTask<UAbilityTask_DropItem>(OwningAbility);
	Task->SlotReference = FromSlot;

	return Task;
}

FGameplayAbilityTargetDataHandle UAbilityTask_DropItem::GenerateTargetHandle()
{
	FGameplayAbilityTargetData_DropItem* ItemSwitchData = new FGameplayAbilityTargetData_DropItem();
	ItemSwitchData->FromSlot = SlotReference;
	

	return FGameplayAbilityTargetDataHandle(ItemSwitchData);
}

void UAbilityTask_DropItem::HandleTargetData(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData_DropItem* SwitchData = static_cast<const FGameplayAbilityTargetData_DropItem*>(Data.Get(0));
	if (SwitchData != nullptr)
	{
		OnDataRecieved.Broadcast(SwitchData->FromSlot);
	}
	else
	{
		OnDataCancelled.Broadcast();
	}
}

void UAbilityTask_DropItem::HandleCancelled()
{
	OnDataCancelled.Broadcast();
}
