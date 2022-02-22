#include "Inventory/ARTInventoryItemTypes.h"
#include "Inventory/Component/ARTInventoryComponent.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/Mod/ARTItemStack_SlotContainer.h"

const int32 NAMED_ITEM_SLOT = -1;

FARTItemSlot FARTItemSlot::Invalid = FARTItemSlot();
FARTItemSlotRef FARTItemSlotRef::Invalid = FARTItemSlotRef();

/*bool IsValid(const FARTItemSlotReference& ItemRef)
{
	if (!ItemRef.ParentInventory.IsValid() && !ItemRef.ParentStack.IsValid())
	{
		return false;
	}

	if(ItemRef.ParentStack.IsValid())
	{
		return ItemRef.ParentStack.Get()->IsValidItemSlot(ItemRef);
	}

	return ItemRef.ParentInventory.Get()->IsValidItemSlot(ItemRef);
}*/

bool IsValid(const FARTItemSlotRef& ItemRef)
{
	if(ItemRef.SlotId < 0) return false;
	
	if (!ItemRef.ParentInventory.IsValid() && !ItemRef.ParentStack.IsValid())
	{
		return false;
	}

	if(ItemRef.ParentStack.IsValid())
	{
		return ItemRef.ParentStack.Get()->IsValidItemSlot(ItemRef);
	}

	return ItemRef.ParentInventory.Get()->IsValidItemSlot(ItemRef);
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

/////////////////////////////////////////////////////////////////

bool FARTSlotQuery::MatchesSlot(const FARTItemSlot& ItemSlot)	const
{
	//Check to see if we have a matching slot type
	if (!SlotTypeQuery.IsEmpty())
	{
		if(!SlotTypeQuery.Matches(ItemSlot.SlotTags)) return false;						 		
	}	

	if (!ItemTypeQuery.IsEmpty())
	{
		//If we are looking for a specific item and this slot doesn't have any item, it's a failed match
		if (!::IsValid(ItemSlot.ItemStack)) return false;

		FGameplayTagContainer StackContainer;
		ItemSlot.ItemStack->GetOwnedGameplayTags(StackContainer);

		if(!ItemTypeQuery.Matches(StackContainer)) return false;
	}
	return true;
}

FARTSlotQuery FARTSlotQuery::QuerySlotMatchingTag(FGameplayTag Tag)
{
	FARTSlotQuery Query;
	Query.SlotTypeQuery = FGameplayTagQuery::MakeQuery_MatchTag(Tag);

	return Query;
}

FARTSlotQuery FARTSlotQuery::QueryForMatchingItemTag(FGameplayTag Tag)
{
	FARTSlotQuery Query;
	Query.ItemTypeQuery = FGameplayTagQuery::MakeQuery_MatchTag(Tag);

	return Query;
}

FARTSlotQuery FARTSlotQuery::QueryForSlot(const FGameplayTagQuery& SlotQuery)
{
	FARTSlotQuery Query;
	Query.SlotTypeQuery = SlotQuery;

	return Query;
}

FARTSlotQuery FARTSlotQuery::QueryForItemType(const FGameplayTagQuery& ItemQuery)
{
	FARTSlotQuery Query;
	Query.ItemTypeQuery = ItemQuery;

	return Query;
}

bool FARTSlotQuery::IsValid() const
{
	return !SlotTypeQuery.IsEmpty() || !ItemTypeQuery.IsEmpty();
}

bool FARTSlotQuery_SlotWithItem::MatchesSlot(const FARTItemSlot& ItemSlot) const
{
	// do not accept empty slot
	UARTItemStack* ItemStack = ItemSlot.ItemStack;
	
	if(!ItemStack) return false;
	
	if(ItemDefinition)
	{
		if(ItemDefinition != ItemStack->GetItemDefinition()) return false;
	}

	switch (StackCount.GetValue())
	{
	case EItemStackCount::Type::ISC_NotMaxStack:
		if(ItemStack->GetStackSize() >= ItemStack->GetItemDefinition().GetDefaultObject()->MaxStackSize) return false;
		break;
	case EItemStackCount::Type::ISC_MaxStack:
		if(ItemStack->GetStackSize() != ItemStack->GetItemDefinition().GetDefaultObject()->MaxStackSize) return false;
		break;
	case EItemStackCount::Type::ISC_Ignore:
		break;
	}

	if(!ItemRequiredTags.IsEmpty() || !ItemBlockedTags.IsEmpty())
	{
		FGameplayTagContainer ItemTags;
		ItemStack->GetOwnedGameplayTags(ItemTags);
		if(!ItemRequiredTags.IsEmpty())
		{
			if(!ItemTags.HasAll(ItemRequiredTags)) return false;
		}
		if(!ItemBlockedTags.IsEmpty())
		{
			if(!ItemTags.HasAny(ItemBlockedTags)) return false;
		}
	}
	return true;
}

bool FARTSlotQuery_SlotCanAcceptItem::MatchesSlot(const FARTItemSlot& ItemSlot) const
{
	UARTItemStack* ItemStack = ItemSlot.ItemStack;
	switch(ItemExist.GetValue())
	{
	case EItemExistence::Type::IE_Empty:
		if(ItemStack) return false;
		break;
	case EItemExistence::Type::IE_HasItem:
		if(!ItemStack) return false;
		break;
	case EItemExistence::Type::IE_Any:
		break;
	}
	if(ContextItemStack)
	{
		if(!ItemSlot.ItemSlotFilter.AcceptsItem(ContextItemStack)) return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////

void FARTItemSlot::ToDebugStrings(TArray<FString>& OutStrings, bool Detailed) const
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

bool FARTItemSlot::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	Ar << SlotId;
	Ar << ItemStack;
	SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	ItemSlotFilter.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}

void FARTItemSlot::PreReplicatedRemove(const struct FARTItemSlotArray& InArraySerializer)
{
	
	ParentStack = InArraySerializer.ParentStack;
	if(ParentStack) ParentStack->PostContainerUpdate();
	
	Owner = InArraySerializer.Owner;
	if(Owner)Owner->PostInventoryUpdate();

	if (IsValid(ItemStack))
	{
		if(Owner) Owner->OnItemSlotChange.Broadcast(Owner, FARTItemSlotRef(*this, Owner), nullptr, ItemStack);
		if(ParentStack) ParentStack->OnContainerSlotUpdate.Broadcast(ParentStack, FARTItemSlotRef(*this, ParentStack), nullptr, ItemStack);
	}
	OldItemStack = nullptr;
}

void FARTItemSlot::PostReplicatedAdd(const struct FARTItemSlotArray& InArraySerializer)
{
	ParentStack = InArraySerializer.ParentStack;
	if(ParentStack) ParentStack->PostContainerUpdate();
	
	Owner = InArraySerializer.Owner;
	if(Owner)Owner->PostInventoryUpdate();

	if (IsValid(ItemStack))
	{
		if(Owner) Owner->OnItemSlotChange.Broadcast(Owner, FARTItemSlotRef(*this, Owner), ItemStack, nullptr);
		if(ParentStack) ParentStack->OnContainerSlotUpdate.Broadcast(ParentStack, FARTItemSlotRef(*this, ParentStack), ItemStack, nullptr);
	}
	OldItemStack = ItemStack;
}

void FARTItemSlot::PostReplicatedChange(const struct FARTItemSlotArray& InArraySerializer)
{
	ParentStack = InArraySerializer.ParentStack;
	if(ParentStack) ParentStack->PostContainerUpdate();
	
	Owner = InArraySerializer.Owner;
	if(Owner)Owner->PostInventoryUpdate();
	
	if(Owner) Owner->OnItemSlotChange.Broadcast(Owner, FARTItemSlotRef(*this, Owner), ItemStack, OldItemStack.Get());
	if(ParentStack) ParentStack->OnContainerSlotUpdate.Broadcast(ParentStack, FARTItemSlotRef(*this, ParentStack), ItemStack, OldItemStack.Get());
		
	OldItemStack = ItemStack;	
}

/////////////////////////////////////////////////////////////////
/*bool FARTContainerItemSlotReference::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << SlotId;
	SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	Ar << ParentStack;

	bOutSuccess = true;
	return true;
}

FString FARTContainerItemSlotReference::ToString() const
{
	return FString::Printf(TEXT("Slot(%d)(%s)-%s"), SlotId, *SlotTags.ToString(), ParentStack ? *ParentStack->GetName() : TEXT("nullptr"));
}

void FARTContainerItemSlot::ToDebugStrings(TArray<FString>& OutStrings, bool Detailed) const
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

bool FARTContainerItemSlot::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	Ar << SlotId;
	Ar << ItemStack;
	SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	ItemSlotFilter.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}

void FARTContainerItemSlot::PreReplicatedRemove(const FARTContainerItemSlotArray& InArraySerializer)
{
	
}

void FARTContainerItemSlot::PostReplicatedAdd(const FARTContainerItemSlotArray& InArraySerializer)
{
}

void FARTContainerItemSlot::PostReplicatedChange(const FARTContainerItemSlotArray& InArraySerializer)
{
}*/

FARTItemSlotRef::FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory)
{
	SlotId = FromSlot.SlotId;
	SlotTags = FromSlot.SlotTags;
	ParentInventory = InParentInventory;
}

FARTItemSlotRef::FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTItemStack_SlotContainer* InParentStack)
{
	SlotId = FromSlot.SlotId;
	SlotTags = FromSlot.SlotTags;
	ParentStack = InParentStack;
}

FARTItemSlotRef::FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory,
	UARTItemStack_SlotContainer* InParentStack)
{
	SlotId = FromSlot.SlotId;
	SlotTags = FromSlot.SlotTags;
	ParentInventory = InParentInventory;
	ParentStack = InParentStack;
}

bool FARTItemSlotRef::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	{
		uint32 RepBits = 0;

		if (Ar.IsSaving())
		{
			if(SlotId > -1)
			{
				RepBits |= 1 << 0;
			}
			if(ParentInventory.IsValid())
			{
				RepBits |= 1 << 1;
			}
			if(ParentStack.IsValid())
			{
				RepBits |= 1 << 2;
			}
		}

		Ar.SerializeBits(&RepBits, 3);
		SlotTags.NetSerialize(Ar, Map, bOutSuccess);
	
		if (RepBits & (1 << 0))
		{
			Ar << SlotId;
		}
		if (RepBits & (1 << 1))
		{
			Ar << ParentInventory;
		}
		if (RepBits & (1 << 2))
		{
			Ar << ParentStack;
		}
	
		bOutSuccess = true;
		return true;
	}
}

FString FARTItemSlotRef::ToString() const
{
	return FString::Printf(TEXT("Slot(%d)(%s)-%s-%s"),
		SlotId,
		*SlotTags.ToString(),
		ParentInventory.IsValid() ? *ParentInventory.Get()->GetName() : TEXT("nullptr"),
		ParentStack.IsValid() ? *ParentStack.Get()->GetName() : TEXT("nullptr"));
}


