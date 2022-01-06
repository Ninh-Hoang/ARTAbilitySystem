// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/InventoryComponent.h"
#include "Item/Item.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#define LOCTEXT_NAMESPACE "Inventory"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	Capacity = 20;
	// ...
}

FItemAddResult UInventoryComponent::TryAddItem(UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<UItem> ItemClass, const int32 Quantity)
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
}

int32 UInventoryComponent::ConsumeItem(UItem* Item, const int32 Quantity)
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
}

bool UInventoryComponent::HasItem(TSubclassOf<UItem> ItemClass, const int32 Quantity) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(UItem* Item) const
{
	if (Item)
	{
		for (UItem* InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<UItem> ItemClass) const
{
	for (UItem* InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;
	for (UItem* InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}
	return ItemsOfClass;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.0;

	for (UItem* Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
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

UItem* UInventoryComponent::AddItem(UItem* Item)
{
	if (GetOwner()->HasAuthority())
	{
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->SetQuantity(Item->GetQuantity());
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();

		return NewItem;
	}

	return nullptr;
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 AddAmount = Item->GetQuantity();

		//check if inventory capacity full
		if (Items.Num() + 1 > GetCapacity())
		{
			return FItemAddResult::AddedNone(
				AddAmount, LOCTEXT("InventoryCapacityFullText", "Could not add item. Inventory is full."));
		}

		//if item weight is 0, don't check weight capacity
		if (FMath::IsNearlyZero(Item->Weight))
		{
			//not enough weight for one item
			if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
			{
				return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryTooMuchWeight",
				                                                    "Could not add item. Exceed inventory weight capacity"));
			}
		}

		//check if item is stackable, if we already have a stack and add it there
		if (Item->bStackable)
		{
			//check if item quantity is over max stack size
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			//if there is already a stack of this item
			if (UItem* ExistingItem = FindItem(Item))
			{
				//if max stack not reached
				if (ExistingItem->GetQuantity() < ExistingItem->MaxStackSize)
				{
					const int32 CapacityMaxAddAmount = ExistingItem->MaxStackSize - ExistingItem->GetQuantity();
					int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

					FText ErrorText = LOCTEXT("InventoryErrorText", "Could not add all items.");

					//adjust if the weight is exceeding capacity
					if (!FMath::IsNearlyZero(Item->Weight))
					{
						//find max amount of item can take due to weight
						const int32 WeightMaxAddAmount = FMath::FloorToInt(
							(WeightCapacity - GetCurrentWeight() / Item->Weight));
						ActualAddAmount = FMath::Min(ActualAddAmount, WeightMaxAddAmount);

						if (ActualAddAmount < AddAmount)
						{
							ErrorText = FText::Format(
								LOCTEXT("InventoryTooMuchWeightText",
								        "Could not entire stack of {ItemName} to Inventory."), Item->ItemDisplayName);
						}
					}
					else if (ActualAddAmount < AddAmount)
					{
						ErrorText = FText::Format(
							LOCTEXT("InventoryTooManyStacksText", "Could not entire stack of {ItemName} to Inventory."),
							Item->ItemDisplayName);
					}

					//if could not add any item
					if (ActualAddAmount <= 0)
					{
						return FItemAddResult::AddedNone(
							AddAmount, LOCTEXT("InventoryErrorText", "Could not add any item to inventory"));
					}

					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);

					ensure(ExistingItem->GetQuantity() <= ExistingItem->MaxStackSize);

					if (ActualAddAmount < AddAmount)
					{
						return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
					}
					return FItemAddResult::AddedAll(AddAmount);
				}
				return FItemAddResult::AddedNone(AddAmount, FText::Format(
					                                 LOCTEXT("InventoryFullStackText",
					                                         "Could not add {ItemName}. Already max stack."),
					                                 Item->ItemDisplayName));
			}
			AddItem(Item);

			return FItemAddResult::AddedAll(AddAmount);
		}
		ensure(Item->GetQuantity() == 1);

		AddItem(Item);

		return FItemAddResult::AddedAll(AddAmount);
	}

	//should not be called on client
	/*if (Item) {
		check(false);
	}*/
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
                                              FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (UItem* Item : Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}
	return bWroteSomething;
}

#undef LOCTEXT_NAMESPACE
