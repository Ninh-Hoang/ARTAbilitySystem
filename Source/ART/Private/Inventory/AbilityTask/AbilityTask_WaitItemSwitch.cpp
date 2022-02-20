// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/AbilityTask/AbilityTask_WaitItemSwitch.h"

#include "Inventory/ARTItemBPFunctionLibrary.h"
#include "Inventory/Component/ARTInventoryComponent_Active.h"

UAbilityTask_WaitItemSwitch::UAbilityTask_WaitItemSwitch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAbilityTask_WaitItemSwitch* UAbilityTask_WaitItemSwitch::WaitActiveItemSwitch(UGameplayAbility* OwningAbility, int32 ItemIndex)
{
	UAbilityTask_WaitItemSwitch* Task = NewAbilityTask<UAbilityTask_WaitItemSwitch>(OwningAbility);
	Task->ItemIndex = ItemIndex;

	return Task;
}

void UAbilityTask_WaitItemSwitch::Activate()
{	 	
	//TODO: Animations here
	//for now, lets just wait a small amount of time and switch weapons

	AActor* Actor = GetAvatarActor();
	if (!IsValid(Actor))
	{
		return;
	}

	Inventory = UARTItemBPFunctionLibrary::GetActiveInventoryComponent(GetAvatarActor());
	if (!IsValid(Inventory))
	{
		return;
	}
	Inventory->bSwitchingWeapons = true;

	Inventory->MakeItemInactive();

	

	Actor->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_WaitItemSwitch::DoItemEquip, .25f, false, .25f);


}

void UAbilityTask_WaitItemSwitch::BeginDestroy()
{
	if (IsValid(Inventory))
	{
		Inventory->bSwitchingWeapons = false;
	}

	Super::BeginDestroy();
}

void UAbilityTask_WaitItemSwitch::DoItemEquip()
{
	if (!IsValid(Inventory))
	{
		return;
	}

	Inventory->MakeItemActive(ItemIndex);
	Inventory->bSwitchingWeapons = false;

	OnItemSwitched.Broadcast();
}