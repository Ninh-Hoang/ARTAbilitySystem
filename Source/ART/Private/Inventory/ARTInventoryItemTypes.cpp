#include "Inventory/ARTInventoryItemTypes.h"
#include "Inventory/Component/ARTInventoryComponent.h"
#include "Inventory/ARTItemStack.h"

const int32 NAMED_ITEM_SLOT = -1;

FARTInventoryItemSlot FARTInventoryItemSlot::Invalid = FARTInventoryItemSlot();
FARTInventoryItemSlotReference FARTInventoryItemSlotReference::Invalid = FARTInventoryItemSlotReference();


bool IsValid(const FARTInventoryItemSlotReference& ItemRef)
{
	if (!IsValid(ItemRef.ParentInventory))
	{
		return false;
	}

	return ItemRef.ParentInventory->IsValidItemSlot(ItemRef);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FARTItemSlotFilter::AcceptsItem(UARTItemStack* ItemStack) const
{
	if (!IsValid(ItemStack))
	{
		return false;
	}

	if (!ItemStack->HasValidItemData())
	{
		return false;
	}

	if (ForceSingleStack)
	{
		if (ItemStack->GetStackSize() > 1)
		{
			return false;
		}		
	}

	FGameplayTagContainer StackTags;
	ItemStack->GetOwnedGameplayTags(StackTags);
	if (!FilterQuery.IsEmpty())
	{
		if (!FilterQuery.Matches(StackTags))
		{
			return false;
		}
	}	

	return true;
}

bool FARTItemSlotFilter::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayTagQuery::StaticStruct()->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, &FilterQuery);
	Ar << ForceSingleStack;

	bOutSuccess = true;
	return true;
}

bool FARTItemSlotFilterHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 RepBits = 0;

	bOutSuccess = true;

	if (Ar.IsSaving())
	{
		if (Data.IsValid())
		{
			RepBits |= 1 << 0;
		}
	}

	Ar.SerializeBits(&RepBits, 1);

	if (RepBits & (1 << 0))
	{
		if (Ar.IsLoading())
		{
			Data = MakeShareable(new FARTItemSlotFilter());
		}

		FARTItemSlotFilter::StaticStruct()->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
	}

	return true;
}

bool FARTInventoryItemSlotReference::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << SlotId;
	SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	Ar << ParentInventory;

	bOutSuccess = true;
	return true;
}

FString FARTInventoryItemSlotReference::ToString() const
{
	return FString::Printf(TEXT("Slot(%d)(%s)-%s"), SlotId, *SlotTags.ToString(), ParentInventory ? *ParentInventory->GetName() : TEXT("nullptr"));
}

void FARTInventoryItemSlot::ToDebugStrings(TArray<FString>& OutStrings, bool Detailed) const
{
	FString SlotName = FString::Printf(TEXT("Slot: %d %s"), SlotId, *SlotTags.ToString()); 
	OutStrings.Add(SlotName);
	if (Detailed)
	{
		//TODO: Add the slot filter here if we are detailed
		if (ItemSlotFilter.IsValid())
		{
			OutStrings.Add(TEXT("Filtered"));
		}
	}
	if (IsValid(ItemStack))
	{
		ItemStack->GetDebugStrings(OutStrings, Detailed);
	}
}

bool FARTInventoryItemSlot::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	Ar << SlotId;
	Ar << ItemStack;
	SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	ItemSlotFilter.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}

void FARTInventoryItemSlot::PreReplicatedRemove(const struct FARTInventoryItemSlotArray& InArraySerializer)
{
	Owner = InArraySerializer.Owner;
	Owner->PostInventoryUpdate();

	if (IsValid(ItemStack))
	{
		Owner->OnItemSlotChange.Broadcast(Owner, FARTInventoryItemSlotReference(*this, Owner), nullptr, ItemStack);
	}
	OldItemStack = nullptr;
}

void FARTInventoryItemSlot::PostReplicatedAdd(const struct FARTInventoryItemSlotArray& InArraySerializer)
{
	Owner = InArraySerializer.Owner;
	Owner->PostInventoryUpdate();

	if (IsValid(ItemStack))
	{
		Owner->OnItemSlotChange.Broadcast(Owner, FARTInventoryItemSlotReference(*this, Owner), ItemStack, nullptr);
	}
	OldItemStack = ItemStack;
}

void FARTInventoryItemSlot::PostReplicatedChange(const struct FARTInventoryItemSlotArray& InArraySerializer)
{
	Owner = InArraySerializer.Owner;
	Owner->PostInventoryUpdate();
	
	Owner->OnItemSlotChange.Broadcast(Owner, FARTInventoryItemSlotReference(*this, Owner), ItemStack, OldItemStack.Get());
	OldItemStack = ItemStack;	
}

/////////////////////////////////////////////////////////////////

bool FARTItemQuery::MatchesSlot(const FARTInventoryItemSlot& ItemSlot)	const
{
	//Slot Tag Matching
	bool Matches = true;

	//Check to see if we have a matching slot type
	if (!SlotTypeQuery.IsEmpty())
	{
		Matches &= SlotTypeQuery.Matches(ItemSlot.SlotTags); 						 		
	}	

	if (!ItemTypeQuery.IsEmpty())
	{
		//If we are looking for a specific item and this slot doesn't have any item, it's a failed match
		if (!::IsValid(ItemSlot.ItemStack))
		{
			return false;
		}

		FGameplayTagContainer StackContainer;
		ItemSlot.ItemStack->GetOwnedGameplayTags(StackContainer);

		Matches &= ItemTypeQuery.Matches(StackContainer);
	}

	return Matches;
}

FARTItemQuery FARTItemQuery::QuerySlotMatchingTag(FGameplayTag Tag)
{
	FARTItemQuery Query;
	Query.SlotTypeQuery = FGameplayTagQuery::MakeQuery_MatchTag(Tag);

	return Query;
}

FARTItemQuery FARTItemQuery::QueryForMatchingItemTag(FGameplayTag Tag)
{
	FARTItemQuery Query;
	Query.ItemTypeQuery = FGameplayTagQuery::MakeQuery_MatchTag(Tag);

	return Query;
}

FARTItemQuery FARTItemQuery::QueryForSlot(const FGameplayTagQuery& SlotQuery)
{
	FARTItemQuery Query;
	Query.SlotTypeQuery = SlotQuery;

	return Query;
}

FARTItemQuery FARTItemQuery::QueryForItemType(const FGameplayTagQuery& ItemQuery)
{
	FARTItemQuery Query;
	Query.ItemTypeQuery = ItemQuery;

	return Query;
}

bool FARTItemQuery::IsValid() const
{
	return !SlotTypeQuery.IsEmpty() || !ItemTypeQuery.IsEmpty();
}

