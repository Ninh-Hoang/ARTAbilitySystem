// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ARTItemStack.h"
#include "Engine/ActorChannel.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/Item/ARTItemRarity.h"
#include "Net/UnrealNetwork.h"

UARTItemStack::UARTItemStack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StackSize = 1;
	ItemName = NSLOCTEXT("ItemStack", "DefaultItemName", "Default Item Name");
	SubItemStacks.ParentStack = this;
}

const int32 Method_Rename = 0;
const int32 Method_Duplicate = 1;

void UARTItemStack::TransferStackOwnership(UARTItemStack*& ItemStack, AActor* Owner)
{
	const int32 TransferMethod = Method_Duplicate;

	if (TransferMethod == Method_Rename)
	{		
		ItemStack->Rename(nullptr, Owner);
		//Recursively rename out substacks
		for (FARTSubItemArrayEntry& SubStack : ItemStack->SubItemStacks.Items)
		{
			TransferStackOwnership(SubStack.SubItemStack, Owner);
		}
		ItemStack->SubItemStacks.MarkArrayDirty();
	}
	else if (TransferMethod == Method_Duplicate)
	{
		UARTItemStack* Original = ItemStack;
		ItemStack = DuplicateObject(Original, Owner);
		//We don't duplicate our sub objects here, because PostDuplicate handles it
	}	
}

void UARTItemStack::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARTItemStack, Rarity);
	DOREPLIFETIME(UARTItemStack, ItemDefinition);
	DOREPLIFETIME(UARTItemStack, SubItemStacks);
	DOREPLIFETIME(UARTItemStack, ItemName);
	DOREPLIFETIME(UARTItemStack, StackSize);
}

bool UARTItemStack::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool bWroteSomething = false;

	for (FARTSubItemArrayEntry SubStack : SubItemStacks.Items)
	{
		if (IsValid(SubStack.SubItemStack))
		{
			bWroteSomething |= Channel->ReplicateSubobject(SubStack.SubItemStack, *Bunch, *RepFlags);
			bWroteSomething |= SubStack.SubItemStack->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}
	return bWroteSomething;
}

bool UARTItemStack::IsSupportedForNetworking() const
{
	return true;
}

void UARTItemStack::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (HasValidItemData())
	{
		ItemDefinition->GetOwnedGameplayTags(TagContainer);
	}
	if (IsValid(Rarity))
	{
		Rarity->GetOwnedGameplayTags(TagContainer);
	}
}

void UARTItemStack::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (DuplicateMode == EDuplicateMode::Normal)
	{
		//Duplicate the SubItems

		TArray<UARTItemStack*> OldSubItems;
		GetSubItems(OldSubItems);
		SubItemStacks.Items.Empty(OldSubItems.Num());
		for (auto SubItem : OldSubItems)
		{
			UARTItemStack* NewSubItem = DuplicateObject(SubItem, GetOuter());
			AddSubItemStack(NewSubItem);
		}
	}
}

AActor* UARTItemStack::GetOwner() const
{
	return GetTypedOuter<AActor>();
}

void UARTItemStack::SetParentStack(UARTItemStack* ItemStack)
{
	ParentItemStack = ItemStack;
}

void UARTItemStack::OnRep_Rarity(UARTItemRarity* PreviousRarity)
{
	//Blank, so that subclasses can override this function
}

void UARTItemStack::OnRep_ItemName(FText PreviousItemName)
{
	//Blank, so that subclasses can override this function
}

void UARTItemStack::OnRep_ItemDefinition(UARTItemDefinition* PreviousItemDefinition)
{
	//Blank, so that subclasses can override this function
}

void UARTItemStack::OnRep_StackSize(int32 PreviousStackSize)
{
	//Blank, so that subclasses can override this function
}

UARTItemDefinition* UARTItemStack::GetItemDefinition() const
{
	return ItemDefinition;
}

int32 UARTItemStack::GetStackSize() const
{
	return StackSize;
}

void UARTItemStack::SetStackSize(int32 NewStackSize)
{
	if (!HasValidItemData())
	{
		return;
	}
	const int32 OldStackSize = StackSize; 
	StackSize = FMath::Clamp(NewStackSize, 1, ItemDefinition->MaxStackSize);
	OnStackSizeChanged.Broadcast(this, OldStackSize, StackSize);
}

bool UARTItemStack::HasValidItemData() const
{
	return IsValid(ItemDefinition);
}

bool UARTItemStack::CanStackWith(UARTItemStack* OtherStack) const
{
	if (!IsValid(OtherStack))
	{
		return false;
	}
	if (!HasValidItemData() || !OtherStack->HasValidItemData())
	{
		return false;
	}

	bool bCanStack = (ItemDefinition->MaxStackSize > 1 && GetStackSize() < ItemDefinition->MaxStackSize);
	return bCanStack && ItemDefinition == OtherStack->ItemDefinition;
}

bool UARTItemStack::MergeItemStacks(UARTItemStack* OtherStack)
{
	if (!IsValid(OtherStack))
	{
		return false;
	}
	if (!CanStackWith(OtherStack))
	{
		return false;
	}
	
	const int32 OldStackSize = StackSize;
	const int32 MaxStacks = ItemDefinition->MaxStackSize;

	if (StackSize + OtherStack->StackSize > MaxStacks)
	{
		const int32 Diff = OtherStack->StackSize + StackSize - MaxStacks;
		
		SetStackSize(MaxStacks);
		OtherStack->SetStackSize(Diff);
		return false;
	}
	else
	{
		StackSize = StackSize + OtherStack->StackSize;
		OnStackSizeChanged.Broadcast(this, OldStackSize, StackSize);
		return true;
	}
}

UARTItemStack* UARTItemStack::SplitItemStack(int32 SplitAmount)
{
	if (!HasValidItemData())
	{
		return nullptr;
	}
	//Cant split out more items than we have in this stack
	if (SplitAmount >= StackSize)
	{
		return nullptr;
	}

	//Can't split 0 or less items
	if (SplitAmount <= 0)
	{
		return nullptr;
	}

	const int32 OldStackSize = StackSize;
	UARTItemStack* NewItemStack = DuplicateObject<UARTItemStack>(this, GetOuter());
	NewItemStack->SetStackSize(SplitAmount);

	StackSize -= SplitAmount;
	
	OnStackSizeChanged.Broadcast(this, OldStackSize, StackSize);
	return NewItemStack;
}

bool UARTItemStack::CanAttachTo_Implementation(UARTItemStack* OtherStack)
{
	//By default we can't attach to another item
	return false;
}

bool UARTItemStack::AddSubItemStack(UARTItemStack* SubItemStack)
{
	if (!IsValid(SubItemStack))
	{
		return false;
	}
	if (!HasValidItemData())
	{
		return false;
	}
	//If an item can stack, then it can't have perks
	if (ItemDefinition->MaxStackSize > 1)
	{
		return false;
	}
	if (!SubItemStack->CanAttachTo(this))
	{
		return false;
	}

	if (SubItemStacks.Items.ContainsByPredicate([SubItemStack](auto x) { return x.SubItemStack == SubItemStack; }))
	{
		return false;
	}

	if (Cast<AActor>(GetOuter()))
	{
		if (SubItemStack->GetOuter() != GetOuter())
		{
			TransferStackOwnership(SubItemStack, Cast<AActor>(GetOuter()));
		}
	}
	

	SubItemStack->SetParentStack(this);
	FARTSubItemArrayEntry Entry;
	Entry.SubItemStack = SubItemStack;
	SubItemStacks.Items.Add(Entry);
	SubItemStacks.MarkArrayDirty();

	OnSubStackAdded.Broadcast(this, SubItemStack);

	return true;
}

bool UARTItemStack::RemoveSubItemStack(UARTItemStack* SubItemStack)
{
	if (SubItemStacks.Items.RemoveAll([SubItemStack] (auto x) {return x.SubItemStack == SubItemStack; }) > 0)
	{
		SubItemStacks.MarkArrayDirty();
		OnSubStackRemoved.Broadcast(this, SubItemStack);

		SubItemStack->SetParentStack(nullptr);
		return true;
	}
	return false;
}

UARTItemUIData_ItemDefinition* UARTItemStack::GetUIData()
{
	return ItemDefinition->UIData;
}

UARTItemStack* UARTItemStack::QueryForSubItem(const FGameplayTagQuery& StackQuery)
{
	for (const FARTSubItemArrayEntry& Stack : SubItemStacks.Items)
	{
		FGameplayTagContainer TagContainer;
		Stack.SubItemStack->GetOwnedGameplayTags(TagContainer);

		if (StackQuery.Matches(TagContainer))
		{
			return Stack.SubItemStack;
		}
	}

	return nullptr;
}

void UARTItemStack::GetSubItems(TArray<UARTItemStack*>& SubItemArray)
{
	SubItemArray.Empty(SubItemStacks.Items.Num());
	for (const FARTSubItemArrayEntry& Stack : SubItemStacks.Items)
	{
		SubItemArray.Add(Stack.SubItemStack);
	}
}

void UARTItemStack::GetDebugStrings(TArray<FString>& OutStrings, bool Detailed) const
{
	OutStrings.Add(FString::Printf(TEXT("Name: %s (%s)"), *ItemName.ToString(), *ItemDefinition->GetName()));
	if (Detailed)
	{
		FString RarityName = IsValid(Rarity) ? Rarity->RarityName.ToString() : TEXT("null");
		OutStrings.Add(FString::Printf(TEXT("StackSize: %d, Rarity: %s, SubItems: %d"), StackSize, *RarityName, SubItemStacks.Items.Num()));		
	}	
}

void FARTSubItemArrayEntry::PreReplicatedRemove(const struct FARTSubItemStackArray& InArraySerializer)
{
	if (IsValid(InArraySerializer.ParentStack))
	{
		InArraySerializer.ParentStack->OnSubStackRemoved.Broadcast(InArraySerializer.ParentStack, SubItemStack);
	}

	PreviousStack = InArraySerializer.ParentStack;
}

void FARTSubItemArrayEntry::PostReplicatedChange(const struct FARTSubItemStackArray& InArraySerializer)
{
	if (!PreviousStack.IsValid() && IsValid(InArraySerializer.ParentStack))
	{
		InArraySerializer.ParentStack->OnSubStackAdded.Broadcast(InArraySerializer.ParentStack, SubItemStack);
	}

	PreviousStack = SubItemStack;
}