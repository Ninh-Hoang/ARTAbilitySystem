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
	TArray<UARTItemStack*> NotFullItemStacks;
	
	FARTSlotQuery_SlotWithItem Query;
	Query.ItemDefinition = Item->GetItemDefinition();
	Query.StackCount = EItemStackCount::ISC_NotMaxStack;

	FARTSlotQuery* NewQuery = new FARTSlotQuery_SlotWithItem(Query);
	FARTSlotQueryHandle QueryHandle;
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(NewQuery);

	Query_GetAllItems(QueryHandle, NotFullItemStacks);

	for(UARTItemStack* NotFullItem : NotFullItemStacks)
	{
		if(NotFullItem->MergeItemStacks(Item)) return true;
	}

	//Find the first empty item slot
	for (auto Slot : ItemContainer.Slots)
	{
		FARTItemSlotRef SlotRef(Slot, this);
		if (PlaceItemIntoSlot(Item, SlotRef))
		{
			return true;
		}
	}

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
	if (IsValid(Item) && Cast<AActor>(GetOuter()))
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
	Item->SetParentStack(this);

	ItemContainer.MarkItemDirty(Slot);

	//Inform the world that we have placed this item here
	OnContainerSlotUpdate.Broadcast(this, ItemSlot, Item, PreviousItem);
	
	if (AActor* OwningActor = Cast<AActor>(GetOuter()))
	{
		OwningActor->ForceNetUpdate();
	}
	return true;

}

bool UARTItemStack_SlotContainer::RemoveItemFromContainer(const FARTItemSlotRef& ItemSlot)
{
	//Check if we have a valid item slot
	if (!IsValid(ItemSlot))
	{
		return false;
	}

	FARTItemSlot& Item = GetItemSlot(ItemSlot);
	UARTItemStack* PreviousItem = Item.ItemStack;

	//Check to make sure we have a valid item in this slot
	if (!IsValid(Item.ItemStack))
	{
		return false;
	}

	//then remove the item
	Item.ItemStack = nullptr;

	ItemContainer.MarkItemDirty(Item);
	ItemContainer.MarkArrayDirty();
	
	OnContainerSlotUpdate.Broadcast(this, ItemSlot, Item.ItemStack, PreviousItem);
	PreviousItem->SetParentStack(nullptr);
	
	if (AActor* OwningActor = Cast<AActor>(GetOuter()))
	{
		OwningActor->ForceNetUpdate();
	}

	return true;
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

bool UARTItemStack_SlotContainer::SwapItemSlots(const FARTItemSlotRef& SourceSlot,
	const FARTItemSlotRef& DestSlot)
{
	//Make sure both slots are valid
	//This checks if ParentInventory is valid, which is important later.  
	if (!IsValid(SourceSlot) || !IsValid(DestSlot))
	{
		return false;
	}	

	UARTItemStack_SlotContainer* SourceParentStack = SourceSlot.ParentStack.Get();
	UARTItemStack_SlotContainer* DestinationParentStack = SourceSlot.ParentStack.Get();

	//If neither the source nor the destination is us... what are we even doing here?
	if (SourceParentStack  != this && DestinationParentStack != this)
	{
		return false;
	}

	UARTItemStack* SourceItem = SourceParentStack->GetItemInSlot(SourceSlot);
	UARTItemStack* DestItem = DestinationParentStack->GetItemInSlot(DestSlot);
														 	
	//Ensure that the two slots can hold these items
	if (!DestinationParentStack->AcceptsItem_AssumeEmptySlot(SourceItem, DestSlot) || !SourceParentStack ->AcceptsItem_AssumeEmptySlot(DestItem, SourceSlot))
	{
		return false;
	}

	SourceParentStack->RemoveItemFromContainer(SourceSlot);
	DestinationParentStack->RemoveItemFromContainer(DestSlot);

	SourceParentStack->PlaceItemIntoSlot(DestItem, SourceSlot);
	DestinationParentStack->PlaceItemIntoSlot(SourceItem, DestSlot);

	return true;
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

int32 UARTItemStack_SlotContainer::GetItemCount()
{
	int32 ItemCount = 0;
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (IsValid(ItemSlot.ItemStack))
		{
			ItemCount++;
		}
	}
	return ItemCount;
}

TArray<FARTItemSlotRef> UARTItemStack_SlotContainer::GetAllSlotReferences()
{
	return AllReferences;
}

bool UARTItemStack_SlotContainer::Query_GetAllSlots(const FARTSlotQueryHandle& Query,
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

FARTItemSlotRef UARTItemStack_SlotContainer::Query_GetFirstSlot(const FARTSlotQueryHandle& Query)
{
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (Query.MatchesSlot(ItemSlot))
		{
			return (FARTItemSlotRef(ItemSlot, this));
		}
	}
	return FARTItemSlotRef();
}

void UARTItemStack_SlotContainer::Query_GetAllItems(const FARTSlotQueryHandle& Query, TArray<UARTItemStack*>& OutItems)
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

UARTItemStack* UARTItemStack_SlotContainer::Query_GetFirstItem(const FARTSlotQueryHandle& Query)
{
	for (FARTItemSlot& ItemSlot : ItemContainer.Slots)
	{
		if (!IsValid(ItemSlot.ItemStack))
		{
			continue;
		}

		if (Query.MatchesSlot(ItemSlot))
		{
			return ItemSlot.ItemStack;
		}
	}
	return nullptr;
}

void UARTItemStack_SlotContainer::OnRep_ItemContainer()
{
	PostContainerUpdate();
}
