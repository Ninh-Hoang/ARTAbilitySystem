// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Mod/ARTItemStack_SlotContainer.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "ART/ART.h"

UARTItemStack_SlotContainer::UARTItemStack_SlotContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StackSize = 1;
}

void UARTItemStack_SlotContainer::InitializeContainer()
{
	//this should only be called by the generator
	for (FARTItemSlotDefinition& SlotDef : CustomInventorySlots)
	{
		CreateContainerSlot(SlotDef.Tags, FARTItemSlotFilterHandle(new FARTItemSlotFilter(SlotDef.Filter)));
	}
}

void UARTItemStack_SlotContainer::CreateContainerSlot(const FGameplayTagContainer& SlotTags,
	const FARTItemSlotFilterHandle& Filter)
{
	FARTItemSlot Slot;
	Slot.SlotId = IdCounter;
	Slot.SlotTags = SlotTags;
	Slot.ItemSlotFilter = Filter;
	Slot.ParentStack = this;

	ItemContainer.Slots.Add(Slot);	

	IdCounter++;
	ItemContainer.MarkItemDirty(Slot);
	ItemContainer.MarkArrayDirty();

	PostContainerUpdate();	
}

void UARTItemStack_SlotContainer::RemoveInventorySlot(const FARTItemSlotRef& Slot)
{
	if (!IsValidItemSlot(Slot))
	{
		return;
	}

	FARTItemSlot& ItemSlot = GetItemSlot(Slot);

	ItemContainer.Slots.Remove(ItemSlot);
	ItemContainer.MarkArrayDirty();

	PostContainerUpdate();	
}

void UARTItemStack_SlotContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UARTItemStack_SlotContainer, ItemContainer);
}

bool UARTItemStack_SlotContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = false;
	WroteSomething |= Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto Slot : ItemContainer.Slots)
	{
		if (IsValid(Slot.ItemStack))
		{
			WroteSomething |= Channel->ReplicateSubobject(Slot.ItemStack, *Bunch, *RepFlags);
			WroteSomething |= Slot.ItemStack->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}

	return WroteSomething;
}

void UARTItemStack_SlotContainer::PostContainerUpdate()
{
	AllReferences.Empty(AllReferences.Num() + 1);
	PopulateSlotReferenceArray(AllReferences); 
	
	OnContainerUpdate.Broadcast(this);
}

void UARTItemStack_SlotContainer::PopulateSlotReferenceArray(TArray<FARTItemSlotRef>& RefArray)
{
	for (int i = 0; i < ItemContainer.Slots.Num(); i++)
	{
		FARTItemSlotRef SlotRef(ItemContainer.Slots[i], this);
		RefArray.Emplace(SlotRef);
	}
	
	Items.Add(FARTItemSlotRef(ItemContainer.Slots[0], this));
}

bool UARTItemStack_SlotContainer::LootItem(UARTItemStack* Item)
{
	return false;
}

bool UARTItemStack_SlotContainer::PlaceItemIntoSlot(UARTItemStack* Item, const FARTItemSlotRef& ItemSlot)
{
	if (!AcceptsItem(Item, ItemSlot))
	{
		return false;
	}

	//Kinda Hack: The Item must be owned by our owning actor for it to be replicated as part of that actor.
	//We can't change ownership over the network (UE4 issue), so instead we duplicate it under the hood.  
	//Only do this if the item owner is not the owner of our component.
	if (Cast<AActor>(GetOuter()))
	{
		if (Item->GetOuter() != GetOuter())
		{
			TransferStackOwnership(Item, Cast<AActor>(GetOuter()));
		}
	}

	//Place the item into the slot
	FARTItemSlot& Slot = GetItemSlot(ItemSlot);
	UARTItemStack* PreviousItem = Slot.ItemStack;
	Slot.ItemStack = Item;

	ItemContainer.MarkItemDirty(Slot);

	//Inform the world that we have placed this item here
	OnContainerUpdate.Broadcast(this);
	OnContainerSlotUpdate.Broadcast(this, ItemSlot, Item, PreviousItem);
	
	if (AActor* OwningActor = Cast<AActor>(GetOuter()))
	{
		OwningActor->ForceNetUpdate();
	}
	return true;

}

bool UARTItemStack_SlotContainer::RemoveItemFromContainer(const FARTItemSlotRef& ItemSlot)
{
	return false;
}

bool UARTItemStack_SlotContainer::IsValidItemSlot(const FARTItemSlotRef& Slot)
{
	//Check to see if we contain this reference
	for (const FARTItemSlotRef& Ref : AllReferences)
	{
		if (Slot == Ref)
		{
			return true;
		}
	}

	return false;
}

bool UARTItemStack_SlotContainer::IsValidItemSlotRef(const FARTItemSlotRef& Slot)
{
	return false;
}

FARTItemSlot& UARTItemStack_SlotContainer::GetItemSlot(const FARTItemSlotRef& RefSlot)
{
	check(IsValidItemSlot(RefSlot));

	for (FARTItemSlot& SlotSlot : ItemContainer.Slots)
	{		
		if (RefSlot == SlotSlot)
		{
			ItemContainer.MarkItemDirty(SlotSlot);
			return SlotSlot;
		}
	}

	//We'll never hit this, since we check IsValidItemSlot
	return ItemContainer.Slots[0];
}

bool UARTItemStack_SlotContainer::RemoveAllItemsFromContainer(TArray<UARTItemStack*>& OutItemsRemoved)
{
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (!IsValid(ItemSlot.ItemStack))
		{
			continue;
		}

		OutItemsRemoved.Add(ItemSlot.ItemStack);
		RemoveItemFromContainer(FARTItemSlotRef(ItemSlot, this));
	}

	return true;
}

bool UARTItemStack_SlotContainer::SwapItemSlots(const FARTItemSlotRef& FromSlot,
	const FARTItemSlotRef& ToSlot)
{
	return false;
}

bool UARTItemStack_SlotContainer::AcceptsItem(UARTItemStack* Item, const FARTItemSlotRef& Slot)
{
	if (!AcceptsItem_AssumeEmptySlot(Item, Slot))
	{
		return false;
	}
	
	FARTItemSlot& SlotSlot = GetItemSlot(Slot);

	//We have an item already in here!
	if (IsValid(SlotSlot.ItemStack))
	{
		return false;
	}

	return true;
}

bool UARTItemStack_SlotContainer::AcceptsItem_AssumeEmptySlot(UARTItemStack* Item, const FARTItemSlotRef& Slot)
{
	//First step, ensure that the slot is valid
	if (!IsValidItemSlot(Slot))
	{
		return false;
	}
		
	FARTItemSlot& SlotSlot = GetItemSlot(Slot);

	//Check if we have a filter, and if we do determine if the filter will let the item in
	if (IsValid(SlotSlot.ItemSlotFilter) && IsValid(Item))
	{
		return SlotSlot.ItemSlotFilter.AcceptsItem(Item);
	}

	//No filter and a valid item?  This slot will hold it
	return true;
}

UARTItemStack* UARTItemStack_SlotContainer::GetItemInSlot(const FARTItemSlotRef& Reference)
{
	if (!IsValidItemSlot(Reference))
	{
		return nullptr;
	}

	FARTItemSlot& ItemSlot = GetItemSlot(Reference);
	ItemContainer.MarkItemDirty(ItemSlot);
	return ItemSlot.ItemStack;
}

int32 UARTItemStack_SlotContainer::GetContainerSize()
{
	return ItemContainer.Slots.Num();
}

TArray<FARTItemSlotRef> UARTItemStack_SlotContainer::GetAllSlotReferences()
{
	return AllReferences;
}

bool UARTItemStack_SlotContainer::Query_GetAllSlots(const FARTItemQuery& Query,
	TArray<FARTItemSlotRef>& OutSlotRefs)
{
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (Query.MatchesSlot(ItemSlot))
		{
			OutSlotRefs.Add(FARTItemSlotRef(ItemSlot, this));
		}
	}
	return OutSlotRefs.Num() > 0;
}

FARTItemSlotRef UARTItemStack_SlotContainer::Query_GetFirstSlot(const FARTItemQuery& Query)
{
	TArray<FARTItemSlotRef> OutSlotRefs;
	
	if (!Query_GetAllSlots(Query, OutSlotRefs))
	{
		UE_LOG(LogInventory, Warning, TEXT("Tried to query for %s but didn't find it"), *Query.SlotTypeQuery.GetDescription())
		return FARTItemSlotRef();
	}

	return OutSlotRefs[0];
}

void UARTItemStack_SlotContainer::Query_GetAllItems(const FARTItemQuery& Query, TArray<UARTItemStack*>& OutItems)
{
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (!IsValid(ItemSlot.ItemStack))
		{
			continue;
		}

		if (Query.MatchesSlot(ItemSlot))
		{
			OutItems.Add(ItemSlot.ItemStack);
		}
	}
}

void UARTItemStack_SlotContainer::OnRep_ItemContainer()
{
	PostContainerUpdate();
}
