// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/InventoryComponent.h"
#include "Item/Item.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#define LOCTEXT_NAMESPACE "Inventory"

bool FItemData::ShouldReplicateItemData() const
{
	if (Item && Item->ShouldReplicateItemData(*this))
	{
		return true;
	}

	return false;
}

void FItemData::PostReplicatedAdd(const FItemDataContainer& InArray)
{
	if (InArray.Owner)
	{
		InArray.Owner->OnAddItem(*this);
	}
}

void FItemData::PreReplicatedRemove(const FItemDataContainer& InArray)
{
	if (InArray.Owner)
	{
		InArray.Owner->OnRemoveItem(*this);
	}
}

void FItemData::PostReplicatedChange(const FItemDataContainer& InArray)
{
	if (InArray.Owner)
	{
		InArray.Owner->OnItemDataChanged(*this);
	}
}

FString FItemData::GetDebugString()
{
	return FString::Printf(TEXT("(%s)"), *GetNameSafe(Item));
}

void FItemDataContainer::RegisterOwner(UInventoryComponent* InOwner)
{
	Owner = InOwner;
}

FScopedItemListLock::FScopedItemListLock(UInventoryComponent& InContainer)
	: InventoryComponent(InContainer) 
{
	InventoryComponent.IncrementItemListLock();
}

FScopedItemListLock::~FScopedItemListLock()
{
	InventoryComponent.DecrementItemListLock();
}

FScopedItemAbilityListLock::FScopedItemAbilityListLock(UInventoryComponent& InInventoryComponent, const UItem& InItem)
	: Item(InItem), ItemLock(InInventoryComponent)
{
}

FScopedItemAbilityListLock::~FScopedItemAbilityListLock()
{
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	Capacity = 20;
	// ...
}

void UInventoryComponent::OnRegister()
{
	Super::OnRegister();

	ItemContainer.RegisterOwner(this);
}

void UInventoryComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UInventoryComponent::OnAddItem(FItemData& ItemData)
{
}

void UInventoryComponent::OnRemoveItem(FItemData& ItemData)
{
}

void UInventoryComponent::OnItemDataChanged(FItemData& ItemData)
{
}

void UInventoryComponent::IncrementItemListLock()
{
	ItemScopeLockCount++;
}

void UInventoryComponent::DecrementItemListLock()
{
	if (--ItemScopeLockCount == 0)
	{
		TArray<FItemData, TInlineAllocator<2> > LocalPendingAdds(MoveTemp(ItemPendingAdds));
		for (FItemData& Item : LocalPendingAdds)
		{
			TryAddItemData(Item);
		}

		TArray<FItemData, TInlineAllocator<2> > LocalPendingRemoves(MoveTemp(ItemPendingRemoves));
		for (FItemData& Item : LocalPendingRemoves)
		{
			//RemoveItem(Item);
		}
	}
}

FItemAddResult UInventoryComponent::TryAddItem(const UItem* Item, int32 Quantity)
{
	return TryAddItemData(FItemData(const_cast<UItem*>(Item), Quantity));
}

FItemAddResult UInventoryComponent::TryAddItemData(const FItemData& ItemData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemData.IsValid())
		return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));

	UItem* Item = ItemData.Item;
	const int32 AddAmount = ItemData.GetQuantity();
	bool AddAll = true;
	int32 ActualAddAmount = AddAmount;
	FText ErrorText;

	//if item weight is 0, don't check weight capacity
	if (!FMath::IsNearlyZero(Item->Weight))
	{
		int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight() / Item->Weight));
		ActualAddAmount = FMath::Min(ActualAddAmount, WeightMaxAddAmount);
	}

	if(ActualAddAmount < 1) return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryCapacityMaxText","Could not add item. Exceed inventory weight capacity"));

	if(ActualAddAmount < AddAmount)
	{
		AddAll = false;
		ErrorText = LOCTEXT("InventoryTooMuchWeightText", "Could not add all items.");
	}
	
	if(Item->AllowMultipleStack)
	{
		int32 AmountToAddLeft = ActualAddAmount;
		if(Item->bStackable)
		{
			TArray<FItemData*> ItemWithNotFullStack;
			if(FindAllItemWithNotMaxStack(Item->GetClass(), ItemWithNotFullStack))
			{
				//fill all not full stack, cache the number left after filling all
				for(FItemData* SingleItem : ItemWithNotFullStack)
				{
					if(AmountToAddLeft <= 0) break;
					const int32 CapacityMaxAddAmount = SingleItem->Item->MaxStackSize - SingleItem->Quantity;
					const int32 AddAmoutToThisStack = FMath::Min(AmountToAddLeft, CapacityMaxAddAmount);
					AmountToAddLeft -= AddAmoutToThisStack;
				}
			}
			
			if(AmountToAddLeft > 0)
			{
				int32 AmountOfNewItem = AmountToAddLeft/Item->MaxStackSize;
				const int32 FinalItemStack = AmountToAddLeft%Item->MaxStackSize;

				AmountOfNewItem = FinalItemStack > 0 ? AmountOfNewItem + 1 : AmountOfNewItem;
				
				const int32 ActualAmountOfNewItem = FMath::Min(Capacity-ItemContainer.Num(), AmountOfNewItem);

				if(ActualAmountOfNewItem < AmountOfNewItem)
				{
					AddAll = false;
					ErrorText = LOCTEXT("InventoryExceedCapacityText", "Could not add all items.");
				}
				
				for(int32 i = 0; i < ActualAmountOfNewItem; i++)
				{
					const int32 ThisStackNum = FMath::Min(Item->MaxStackSize, AmountToAddLeft);
					AmountToAddLeft -= ThisStackNum;
					AddItem(Item, ThisStackNum);
				}
			}
			
			ActualAddAmount = ActualAddAmount - AmountToAddLeft;
			if(ActualAddAmount == 0)  return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryCapacityMaxText","Could not add item. Exceed inventory capacity"));
		}
		else
		{
			const int32 ActualAmountOfNewItem = FMath::Min(Capacity-ItemContainer.Num(), AmountToAddLeft);
			
			if(ActualAmountOfNewItem < AmountToAddLeft)
			{
				AddAll = false;
				ErrorText = LOCTEXT("InventoryExceedCapacityText", "Could not add all items.");
			}
			
			for(int32 i = 0; i < ActualAmountOfNewItem; i++)
			{
				AddItem(Item);
			}
			
		}
	}
	else
	{
		if(Item->bStackable)
		{
			FItemData ExistingItem;
			if(FindItem(Item, ExistingItem))
			{
				ActualAddAmount = FMath::Min(ActualAddAmount, Item->MaxStackSize - ExistingItem.Quantity);

				if(ActualAddAmount == 0)
				{
					AddAll = false;
					return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryItemMaxStackText","Could not add item. ItemStack maxed"));
				}
				
				if(ActualAddAmount < AddAmount)
				{
					AddAll = false;
					ErrorText = LOCTEXT("InventoryItemMaxStackText", "Could not add all items.");
				}
				ExistingItem.AddQuantity(ActualAddAmount);
			}
			else
			{
				ActualAddAmount = FMath::Min(ActualAddAmount, Item->MaxStackSize);
				
				if(ActualAddAmount < AddAmount)
				{
					AddAll = false;
					ErrorText = LOCTEXT("InventoryItemMaxStackText", "Could not add all items.");
				}
				
				AddItem(Item, ActualAddAmount);
			}
		}
		else
		{
			FItemData ExistingItem;
			if(FindItem(Item, ExistingItem))
			{
				AddAll = false;
				return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryItemUniqueItemExistText","Could not add item. UniqueItemExist"));
			}
			else
			{
				ActualAddAmount = 1;
				AddItem(Item);

				if(ActualAddAmount < AddAmount)
				{
					AddAll = false;
					ErrorText = LOCTEXT("InventoryItemUniqueItemExistText", "Could not add all items.");
				}
				
			}
		}
	}

	if(AddAll) return FItemAddResult::AddedAll(AddAmount);
	return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
}

/*FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<UItem> ItemClass, const int32 Quantity)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

int32 UInventoryComponent::ConsumeItem(UItem* Item)
{
	if (Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}*/

/*int32 UInventoryComponent::ConsumeItem(UItem* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		//check if result item quantity is negative
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
			ClientRefreshInventory();
		}
		else
		{
			ClientRefreshInventory();
		}

		return RemoveQuantity;
	}
	return 0;
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);
			ReplicatedItemsKey++;
			return true;
		}
	}
	return false;
}*/

bool UInventoryComponent::HasItem(const UItem* SearchItem, const int32 Quantity) const
{
	if(Quantity < 1) return true;
	
	FItemData TempItem;
	if(Quantity == 1)
		return FindItem(SearchItem, TempItem);

	if(Quantity > 1)
	{
		int32 CurrentQuantity = 0;
		for(const FItemData& Item : ItemContainer.Items)
		{
			if(Item.Item == SearchItem)
			{
				CurrentQuantity += Item.Quantity;
				if(CurrentQuantity >= Quantity) return true;
			}
		}
	}
	return false;
}

bool UInventoryComponent::FindItem(const UItem* SearchItem, FItemData& OutItem) const
{
	if(!SearchItem) return false;
	
	for(const FItemData& Item : ItemContainer.Items)
	{
		if(Item.Item  == SearchItem)
		{
			OutItem = Item;
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::FindItems(const UItem* SearchItem, TArray<FItemData*>& OutItems) const
{
	if(!SearchItem) return false;

	bool Found = false;

	for(const FItemData& Item : ItemContainer.Items)
	{
		if(Item.Item  == SearchItem)
		{
			OutItems.Add(const_cast<FItemData*>(&Item));
			Found = true;
		}
	}
	return Found;
}

bool UInventoryComponent::FindAllItemWithNotMaxStack(const TSubclassOf<UItem> ItemClass,
	TArray<FItemData*>& OutItems) const
{
	if(!ItemClass) return false;

	bool Found = false;
	for(const FItemData& Item : ItemContainer.Items)
	{
		if(Item.Item->GetClass() == ItemClass && Item.Quantity < Item.Item->MaxStackSize)
		{
			OutItems.Add(const_cast<FItemData*>(&Item));
			Found = true;
		}
	}
	return Found;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.0;

	for(const FItemData& Item : ItemContainer.Items)
	{
		Weight += Item.Item->Weight;
	}

	return Weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}


void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

FItemData* UInventoryComponent::AddItem(UItem* Item)
{
	if (GetOwner()->HasAuthority())
	{
		const FItemData ItemDataToAdd(Item, Item->MaxStackSize);
		
		FItemData& AddedItemData = ItemContainer.Items[ItemContainer.Items.Add(ItemDataToAdd)];
		
		ItemContainer.MarkItemDirty(AddedItemData );
		return &AddedItemData;
	}
	return nullptr;
}

FItemData* UInventoryComponent::AddItem(UItem* Item, int32 Quantity)
{
	if (GetOwner()->HasAuthority())
	{
		const FItemData ItemDataToAdd(Item, Quantity);
		FItemData& AddedItemData = ItemContainer.Items[ItemContainer.Items.Add(ItemDataToAdd)];
		
		ItemContainer.MarkItemDirty(AddedItemData );
		return &AddedItemData;
	}
	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, ItemContainer);
}


#undef LOCTEXT_NAMESPACE
