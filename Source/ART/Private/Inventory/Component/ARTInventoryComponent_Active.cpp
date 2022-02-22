// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Component/ARTInventoryComponent_Active.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ART/ART.h"
#include "Inventory/ARTItemStack.h"
#include "Inventory/Item/Definition/ARTItemDefinition_Active.h"
#include "Net/UnrealNetwork.h"

UARTInventoryComponent_Active::UARTInventoryComponent_Active(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveItemSlot = INDEX_NONE;
	PendingItemSlot = INDEX_NONE;
}

void UARTInventoryComponent_Active::InitializeComponent()
{
	Super::InitializeComponent();


	UpdateActiveItemSlots(this);
	OnInventoryUpdate.AddDynamic(this, &UARTInventoryComponent_Active::UpdateActiveItemSlots);
}

void UARTInventoryComponent_Active::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTInventoryComponent_Active, ActiveItemSlot, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTInventoryComponent_Active, PendingItemSlot, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION(UARTInventoryComponent_Active, ActiveItemAbilityInfos, COND_OwnerOnly);
}

void UARTInventoryComponent_Active::BeginPlay()
{
	//Make sure we have nothing stored when we begin play.  We want to have a clean start to this active slot if we reset
	int32 OldActiveItem = ActiveItemSlot;
	MakeItemInactive();
	ActiveItemSlot = OldActiveItem;

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		//Check to see if we have an active item in our first slot and set it to that to start with  
		if (PendingItemSlot != INDEX_NONE)
		{
			this->SwitchToPendingItemSlot();
		}
		//Sometimes, on the client, we get the initial ActiveItemSlot before we've begun play
		//In that case, PendingItemSlot would be none, and we have a valid ActiveItemSlot that hasn't been made active yet
		//So we'll do it here.
		else if (ActiveItemSlot != INDEX_NONE && GetOwnerRole() != ROLE_Authority)
		{
			this->MakeItemActive(ActiveItemSlot);
		}
	});

	Super::BeginPlay();
}

void UARTInventoryComponent_Active::OnItemEquipped(class UARTInventoryComponent* Inventory, const FARTItemSlotRef& ItemSlotRef, UARTItemStack* ItemStack, UARTItemStack* PreviousItemStack)
{
	Super::OnItemEquipped(Inventory, ItemSlotRef, ItemStack, PreviousItemStack);

	//If we are an active item slot, make it active if we don't already have an active item		
	if (ActiveItemSlot == INDEX_NONE && IsActiveItemSlot(ItemSlotRef) && IsValid(ItemStack))
	{

		int32 ItemSlotIndex = GetActiveItemIndexBySlotRef(ItemSlotRef);
		PendingItemSlot = ItemSlotIndex;

		//If we've begun play, send the gameplay event now.  Otherwise we'll get it in BeginPlay
		if (HasBegunPlay())
		{
			SwitchToPendingItemSlot();
		}
	}

	//If we are unequipping an item and it's the currently active item, either go to the next available active item or go to neutral	
	const int32 ItemSlotIndex = GetActiveItemIndexBySlotRef(ItemSlotRef);
	if (ActiveItemSlot != INDEX_NONE && ItemSlotIndex == ActiveItemSlot)
	{
		PendingItemSlot = IsValid(ItemStack) ? ItemSlotIndex : GetNextValidActiveItemSlot();
		MakeItemInactive_Internal(ItemSlotRef, PreviousItemStack);
		SwitchToPendingItemSlot();
	}
	
}

FARTItemSlotRef UARTInventoryComponent_Active::GetActiveItemSlot()
{
	return GetActiveItemSlotInSlot(ActiveItemSlot);
}

UARTItemStack* UARTInventoryComponent_Active::GetActiveItemStack()
{
	return GetActiveItemStackInSlot(ActiveItemSlot);
}

FARTItemSlotRef UARTInventoryComponent_Active::GetActiveItemSlotInSlot(int32 InActiveItemSlot)
{
	if (InActiveItemSlot == INDEX_NONE)
	{
		return FARTItemSlotRef();
	}

	FARTItemSlotRef ItemSlotRef = CachedActiveItemSlots[InActiveItemSlot];
	return ItemSlotRef;
}

UARTItemStack* UARTInventoryComponent_Active::GetActiveItemStackInSlot(int32 InActiveItemSlot)
{
	FARTItemSlotRef ItemSlotRef = GetActiveItemSlotInSlot(InActiveItemSlot);
	return GetItemInSlot(ItemSlotRef);
}

bool UARTInventoryComponent_Active::IsValidActiveItemSlot(int32 InActiveItemSlot)
{
	//Do we have a valid index?
	if (InActiveItemSlot < 0)
	{
		return false;
	}
	if (InActiveItemSlot >= CachedActiveItemSlots.Num())
	{
		return false;
	}

	//Is the index a valid item?
	UARTItemStack* ItemStack = GetActiveItemStackInSlot(InActiveItemSlot);
	if (!IsValid(ItemStack))
	{
		return false;
	}

	return true;
}

void UARTInventoryComponent_Active::OnRep_ActiveItemSlot(int32 PreviousItemSlot)
{
	//If we haven't begun play yet, we'll do this on BeginPlay
	//Otherwise, do it as soon as we get the item slot change from the server
	if (HasBegunPlay())
	{
		if (ActiveItemSlot != PreviousItemSlot)
		{
			if (PreviousItemSlot != INDEX_NONE)
			{
				MakeItemInactive_Internal(GetActiveItemSlotInSlot(PreviousItemSlot));
			}
			if (ActiveItemSlot != INDEX_NONE)
			{
				MakeItemActive(ActiveItemSlot);
			}
		}
	}
}

void UARTInventoryComponent_Active::OnRep_PendingItemSlot(int32 PreviousItemSlot)
{

}

int32 UARTInventoryComponent_Active::GetNextActiveItemSlot() const
{
	return GetNextItemSlotFrom(ActiveItemSlot);
}

int32 UARTInventoryComponent_Active::GetPreviousActiveItemSlot() const
{
	return GetPreviousItemSlotFrom(ActiveItemSlot);
}

int32 UARTInventoryComponent_Active::GetNextValidActiveItemSlot() 
{
	int32 TestItemSlot = GetNextActiveItemSlot();

	//Check to make sure we aren't INDEX_NONE or negative.  If we are either of these, then this loop goes infinite.
	int32 StartActiveItemSlot = ActiveItemSlot < 0 ? 0 : ActiveItemSlot;
	while (TestItemSlot != StartActiveItemSlot)
	{
		if (CachedActiveItemSlots.IsValidIndex(TestItemSlot))
		{
			FARTItemSlotRef ItemSlotRef = CachedActiveItemSlots[TestItemSlot];

			if (IsValid(GetItemInSlot(ItemSlotRef)))
			{
				return TestItemSlot;
			}
		}

		TestItemSlot = GetNextItemSlotFrom(TestItemSlot);
	}

	//If we started as INDEX_NONE, then the loop would fall through if there was only one active item.  
	//So, check for that case and return the Start slot if it's valid	
	if (StartActiveItemSlot != ActiveItemSlot && CachedActiveItemSlots.IsValidIndex(StartActiveItemSlot))
	{
		return StartActiveItemSlot;
	}
	else
	{
		return INDEX_NONE;
	}
	
}

int32 UARTInventoryComponent_Active::GetPreviousValidActiveItemSlot()
{
	int32 TestItemSlot = GetPreviousActiveItemSlot();

	//Check to make sure we aren't INDEX_NONE or negative.  If we are either of these, then this loop goes infinite.
	int32 StartActiveItemSlot = ActiveItemSlot < 0 ? 0 : ActiveItemSlot;
	while (TestItemSlot != StartActiveItemSlot)
	{
		if (CachedActiveItemSlots.IsValidIndex(TestItemSlot))
		{
			FARTItemSlotRef ItemSlotRef = CachedActiveItemSlots[TestItemSlot];

			if (IsValid(GetItemInSlot(ItemSlotRef)))
			{
				return TestItemSlot;
			}
		}

		TestItemSlot = GetPreviousItemSlotFrom(TestItemSlot);
	}

	//If we started as INDEX_NONE, then the loop would fall through if there was only one active item.  
	//So, check for that case and return the Start slot if it's valid	
	if (StartActiveItemSlot != ActiveItemSlot && CachedActiveItemSlots.IsValidIndex(StartActiveItemSlot))
	{
		return StartActiveItemSlot;
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 UARTInventoryComponent_Active::GetNextItemSlotFrom(int32 InActiveItemSlot) const
{
	int32 NextItemSlot = InActiveItemSlot + 1;
	if (NextItemSlot >= CachedActiveItemSlots.Num())
	{
		NextItemSlot = 0;
	}

	return NextItemSlot;
}

int32 UARTInventoryComponent_Active::GetPreviousItemSlotFrom(int32 InActiveItemSlot) const
{
	int32 PreviousItemSlot = InActiveItemSlot - 1;
	if (PreviousItemSlot < 0)
	{
		PreviousItemSlot = CachedActiveItemSlots.Num() - 1;
	}

	return PreviousItemSlot;
}

void UARTInventoryComponent_Active::MakeItemInactive()
{
	MakeItemInactive_Internal(GetActiveItemSlot());

	//If we are not actively switching weapons, set the active itemslot to none
	//bSwitchingWeapons is true when the item switch task is doing the switching, and we expect that a new item will become active
	if (/*GetOwnerRole() == ROLE_Authority &&*/ !bSwitchingWeapons)
	{
		ActiveItemSlot = INDEX_NONE;
	}
}

void UARTInventoryComponent_Active::MakeItemActive(int32 NewActiveItemSlot)
{
	if (NewActiveItemSlot == INDEX_NONE)
	{
		return;
	}

	if (!ensure(CachedActiveItemSlots.IsValidIndex(NewActiveItemSlot)))
	{
		return;
	}

	FARTItemSlotRef ItemSlotRef = CachedActiveItemSlots[NewActiveItemSlot];

	

	if (!MakeItemActive_Internal(ItemSlotRef))
	{
		if (IsValid(GetItemInSlot(ItemSlotRef)))
		{
			//If we fail to make the new item active and we have a valid item, then set the active slot to none
			//Swaping to an active item slot without a valid item is a valid action.  Any other failure, reset to INDEX_NONE
			NewActiveItemSlot = INDEX_NONE;
		}		
	}

	//If we are not actively switching weapons, set the active itemslot to none
	//bSwitchingWeapons is true when the item switch task is doing the switching, and we expect that a new item will become active	
	//if (GetOwnerRole() == ROLE_Authority)
	{
		ActiveItemSlot = NewActiveItemSlot;
	}
}

void UARTInventoryComponent_Active::MakeItemActive(const FARTItemSlotRef& ItemSlotRef)
{
	int32 Slot = GetActiveItemIndexBySlotRef(ItemSlotRef);
	MakeItemActive(Slot);
}

bool UARTInventoryComponent_Active::IsActiveItemSlot(const FARTItemSlotRef& ItemSlotRef) const
{
	return ItemSlotRef.SlotTags.HasTagExact(InvActiveSlotTag);
}

int32 UARTInventoryComponent_Active::GetActiveItemIndexBySlotRef(const FARTItemSlotRef& ItemSlotRef)
{
	int32 SlotIndex = INDEX_NONE;
	CachedActiveItemSlots.Find(ItemSlotRef, SlotIndex);

	return SlotIndex;
}

int32 UARTInventoryComponent_Active::GetActiveItemIndexByTag(FGameplayTag Tag)
{
	FARTSlotQueryHandle QueryHandle;
	FARTSlotQuery* Query = new FARTSlotQuery(FARTSlotQuery::QuerySlotMatchingTag(Tag));
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(Query);
	FARTItemSlotRef Ref = Query_GetFirstSlot(QueryHandle);
	return GetActiveItemIndexBySlotRef(Ref);
}

void UARTInventoryComponent_Active::SwitchToPendingItemSlot()
{
	//If we have an Ability System Component on this actor, lets send a gameplay event to allow a GameplayAbility handle the item switching.
	//This will handle animations and timing and all the important bits.
	bool bActivatedAbility = false;
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(GetOwner()))
	{
		bActivatedAbility = ASC->TryActivateAbilitiesByTag(InvSwapPendingAbilityTag.GetTag().GetSingleTagContainer());
	}

	if (!bActivatedAbility)//Otherwise just quickly switch the item
	{
		MakeItemInactive();
		MakeItemActive(PendingItemSlot);		
	}

	PendingItemSlot = INDEX_NONE;
}

void UARTInventoryComponent_Active::UpdateActiveItemSlots(UARTInventoryComponent* InventoryComp)
{
	//Cache the Active Item Slots
	FARTSlotQueryHandle QueryHandle;
	FARTSlotQuery* Query = new FARTSlotQuery(FARTSlotQuery::QuerySlotMatchingTag(InvActiveSlotTag));
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(Query);
	
	CachedActiveItemSlots.Empty(CachedActiveItemSlots.Num());
	Query_GetAllSlots(QueryHandle, CachedActiveItemSlots);
}

void UARTInventoryComponent_Active::SwapActiveItems(int32 NewItemSlot)
{
	bSwitchingWeapons = true;
	MakeItemInactive();
	MakeItemActive(NewItemSlot);
	bSwitchingWeapons = false;
}

int32 UARTInventoryComponent_Active::GetIndexForActiveItemSlotTagQuery(const FGameplayTagQuery& TagQuery)
{
	for (int32 i = 0; i < CachedActiveItemSlots.Num(); i++)
	{
		if (TagQuery.Matches(CachedActiveItemSlots[i].SlotTags))
		{
			return i;
		}
	}

	return INDEX_NONE;
}



bool UARTInventoryComponent_Active::MakeItemInactive_Internal(const FARTItemSlotRef& ItemSlot)
{
	if (!IsActiveItemSlot(ItemSlot))
	{
		return false;
	}

	UARTItemStack* ItemStack = GetItemInSlot(ItemSlot);

	return MakeItemInactive_Internal(ItemSlot, ItemStack);
}

bool UARTInventoryComponent_Active::MakeItemInactive_Internal(const FARTItemSlotRef& ItemSlot, UARTItemStack* ItemStack)
{
	if (!IsValid(ItemStack))
	{
		//If we have a valid item slot, but no valid item stack, lets just call the OnItemInactive event because we have nothing to remove.
		//This is most likely to happen on the client when an item has already been to removed from the slot but we are trying to do the client side removal
		if (IsValid(ItemSlot))
		{
			OnItemInactive.Broadcast(this, ItemStack);
		}
		return false;
	}

	TSubclassOf<UARTItemDefinition_Active> ItemDefinition(ItemStack->GetItemDefinition());
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	FARTInventoryItemInfoEntry* Entry = ActiveItemAbilityInfos.FindByPredicate([ItemSlot](const FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef.SlotId == ItemSlot.SlotId;
	});
	if (Entry == nullptr)
	{
		return false;
	}

	RemoveMods(ItemStack, ItemSlot);
	bool bSuccess = ClearAbilityInfo_Internal(ItemDefinition.GetDefaultObject()->ActiveItemAbilityInfo, (*Entry).EquippedItemInfo);
		

	OnItemInactive.Broadcast(this, ItemStack);

	return bSuccess;
}

bool UARTInventoryComponent_Active::MakeItemActive_Internal(const FARTItemSlotRef& ItemSlot)
{
	if (!IsActiveItemSlot(ItemSlot))
	{
		return false;
	}

	UARTItemStack* ItemStack = GetItemInSlot(ItemSlot);

	if (!IsValid(ItemStack))
	{
		return false;
	}

	return MakeItemActive_Internal(ItemSlot, ItemStack);
}

bool UARTInventoryComponent_Active::MakeItemActive_Internal(const FARTItemSlotRef& ItemSlot, UARTItemStack* ItemStack)
{
	if (!IsValid(ItemStack))
	{
		return false;
	}

	TSubclassOf<UARTItemDefinition_Active> ItemDefinition(ItemStack->GetItemDefinition());
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	FARTInventoryItemInfoEntry* Entry = ActiveItemAbilityInfos.FindByPredicate([ItemSlot](const FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef.SlotId == ItemSlot.SlotId;
	});
	if (Entry == nullptr)
	{
		Entry = &ActiveItemAbilityInfos.Add_GetRef(FARTInventoryItemInfoEntry(ItemSlot));
	}

	//Add this item's Active Abilities
	bool bSuccess = ApplyAbilityInfo_Internal(ItemDefinition.GetDefaultObject()->ActiveItemAbilityInfo, (*Entry).EquippedItemInfo, ItemStack);

	if (bSuccess)
	{
		ApplyMods(ItemStack, ItemSlot);		
	}

	OnItemActive.Broadcast(this, ItemStack);

	return bSuccess;
}