// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Mod/ARTItemStack_SlotContainer.h"

UARTItemStack_SlotContainer::UARTItemStack_SlotContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UARTItemStack_SlotContainer::InitializeItemStack()
{
}

void UARTItemStack_SlotContainer::CreateContainerSlot(const FGameplayTagContainer& SlotTags,
	const FARTItemSlotFilterHandle& Filter)
{
}

void UARTItemStack_SlotContainer::RemoveInventorySlot(const FARTInventoryItemSlotReference& Slot)
{
}

void UARTItemStack_SlotContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UARTItemStack_SlotContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}
