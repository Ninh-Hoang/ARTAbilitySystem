// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Component/ARTInventoryComponent.h"
#include "ART/ART.h"
#include "AbilitySystemGlobals.h"
#include "DisplayDebugHelpers.h"
#include "Inventory/ARTItemStack.h"
#include "Engine/Canvas.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

// Sets default values for this component's properties
UARTInventoryComponent::UARTInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	IdCounter = 0;

}

void UARTInventoryComponent::PostInitProperties()
{
	BagInventory.Owner = this;
	Super::PostInitProperties();
}

void UARTInventoryComponent::InitializeComponent()
{	
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (FARTItemSlotDefinition& SlotDef : CustomInventorySlots)
		{
			CreateInventorySlot(SlotDef.Tags, FARTItemSlotFilterHandle(new FARTItemSlotFilter(SlotDef.Filter)));
		}
	}
	
	
	Super::InitializeComponent();
}

void UARTInventoryComponent::CreateInventorySlot(const FGameplayTagContainer& SlotTags, const FARTItemSlotFilterHandle& Filter)
{
	FARTInventoryItemSlot Slot;
	Slot.SlotId = IdCounter;
	Slot.SlotTags = SlotTags;
	Slot.ItemSlotFilter = Filter;
	Slot.Owner = this;

	BagInventory.Slots.Add(Slot);	

	IdCounter++;
	BagInventory.MarkItemDirty(Slot);
	BagInventory.MarkArrayDirty();

	PostInventoryUpdate();	
}

void UARTInventoryComponent::RemoveInventorySlot(const FARTInventoryItemSlotReference& Slot)
{
	if (!IsValidItemSlot(Slot))
	{
		return;
	}

	FARTInventoryItemSlot& ItemSlot = GetItemSlot(Slot);

	BagInventory.Slots.Remove(ItemSlot);
	BagInventory.MarkArrayDirty();

	PostInventoryUpdate();
}

void UARTInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARTInventoryComponent, BagInventory);
}

bool UARTInventoryComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = false;
	WroteSomething |= Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto Slot : BagInventory.Slots)
	{
		if (IsValid(Slot.ItemStack))
		{
			WroteSomething |= Channel->ReplicateSubobject(Slot.ItemStack, *Bunch, *RepFlags);
			WroteSomething |= Slot.ItemStack->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}

	return WroteSomething;
}

bool UARTInventoryComponent::LootItem(UARTItemStack* Item)
{
	//We can't do this on clients
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	//Find the first empty item slot
	for (auto Slot : BagInventory.Slots)
	{
		FARTInventoryItemSlotReference SlotRef(Slot, this);
		if (AcceptsItem(Item, SlotRef) && PlaceItemIntoSlot(Item, SlotRef))
		{
			return true;
		}
	}

	return false;
}

bool UARTInventoryComponent::PlaceItemIntoSlot(UARTItemStack* Item, const FARTInventoryItemSlotReference& ItemSlot)
{
	//We can't do this on clients
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}
	
	if (!AcceptsItem(Item, ItemSlot))
	{
		return false;
	}

	//Kinda Hack: The Item must be owned by our owning actor for it to be replicated as part of that actor.
	//We can't change ownership over the network (UE4 issue), so instead we duplicate it under the hood.  
	//Only do this if the item owner is not the owner of our component.
	if (IsValid(Item) && Item->GetOwner() != GetOwner())
	{
		UARTItemStack::TransferStackOwnership(Item, GetOwner());
	}

	//Place the item into the slot
	FARTInventoryItemSlot& Slot = GetItemSlot(ItemSlot);
	UARTItemStack* PreviousItem = Slot.ItemStack;
	Slot.ItemStack = Item;

	BagInventory.MarkItemDirty(Slot);

	//Inform the world that we have placed this item here
	OnInventoryUpdate.Broadcast(this);
	OnItemSlotChange.Broadcast(this, ItemSlot, Item, PreviousItem);
	
	GetOwner()->ForceNetUpdate();
	return true;
}

bool UARTInventoryComponent::RemoveItemFromInventory(const FARTInventoryItemSlotReference& ItemSlot)
{
	//We can't do this on clients
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	//Check if we have a valid item slot
	if (!IsValid(ItemSlot))
	{
		return false;
	}

	FARTInventoryItemSlot& Item = GetItemSlot(ItemSlot);
	UARTItemStack* PreviousItem = Item.ItemStack;

	//Check to make sure we have a valid item in this slot
	if (!IsValid(Item.ItemStack))
	{
		return false;
	}

	//then remove the item
	Item.ItemStack = nullptr;

	BagInventory.MarkItemDirty(Item);
	BagInventory.MarkArrayDirty();

	OnInventoryUpdate.Broadcast(this);
	OnItemSlotChange.Broadcast(this, ItemSlot, Item.ItemStack, PreviousItem);

	GetOwner()->ForceNetUpdate();

	return true;
}

bool UARTInventoryComponent::RemoveAllItemsFromInventory(TArray<UARTItemStack*>& OutItemsRemoved)
{
	for (FARTInventoryItemSlot& ItemSlot : BagInventory.Slots)
	{
		if (!IsValid(ItemSlot.ItemStack))
		{
			continue;
		}

		OutItemsRemoved.Add(ItemSlot.ItemStack);
		RemoveItemFromInventory(FARTInventoryItemSlotReference(ItemSlot, this));
	}

	return true;
}

bool UARTInventoryComponent::SwapItemSlots(const FARTInventoryItemSlotReference& SourceSlot, const FARTInventoryItemSlotReference& DestSlot)
{
	//If we aren't the server, Call the server RPC
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}


	//Make sure both slots are valid
	//This checks if ParentInventory is valid, which is important later.  
	if (!IsValid(SourceSlot) || !IsValid(DestSlot))
	{
		return false;
	}	

	UARTInventoryComponent* SourceInventory = SourceSlot.ParentInventory;
	UARTInventoryComponent* DestinationInventory = DestSlot.ParentInventory;

	//If neither the source nor the destination is us... what are we even doing here?
	if (SourceInventory != this && DestinationInventory != this)
	{
		return false;
	}

	UARTItemStack* SourceItem = SourceInventory->GetItemInSlot(SourceSlot);
	UARTItemStack* DestItem = DestinationInventory->GetItemInSlot(DestSlot);
														 	
	//Ensure that the two slots can hold these items
	if (!DestinationInventory->AcceptsItem_AssumeEmptySlot(SourceItem, DestSlot) || !SourceInventory->AcceptsItem_AssumeEmptySlot(DestItem, SourceSlot))
	{
		return false;
	}

	SourceInventory->RemoveItemFromInventory(SourceSlot);
	DestinationInventory->RemoveItemFromInventory(DestSlot);

	SourceInventory->PlaceItemIntoSlot(DestItem, SourceSlot);
	DestinationInventory->PlaceItemIntoSlot(SourceItem, DestSlot);

	return true;
}

bool UARTInventoryComponent::AcceptsItem(UARTItemStack* Item, const FARTInventoryItemSlotReference& Slot)
{
	
	if (!AcceptsItem_AssumeEmptySlot(Item, Slot))
	{
		return false;
	}
	
	FARTInventoryItemSlot& SlotSlot = GetItemSlot(Slot);

	//We have an item already in here!
	if (IsValid(SlotSlot.ItemStack))
	{
		return false;
	}

	return true;
}

bool UARTInventoryComponent::AcceptsItem_AssumeEmptySlot(UARTItemStack* Item, const FARTInventoryItemSlotReference& Slot)
{
	//First step, ensure that the slot is valid
	if (!IsValidItemSlot(Slot))
	{
		return false;
	}
		
	FARTInventoryItemSlot& SlotSlot = GetItemSlot(Slot);

	//Check if we have a filter, and if we do determine if the filter will let the item in
	if (IsValid(SlotSlot.ItemSlotFilter) && IsValid(Item))
	{
		return SlotSlot.ItemSlotFilter.AcceptsItem(Item);
	}

	//No filter and a valid item?  This slot will hold it
	return true;
}

UARTItemStack* UARTInventoryComponent::GetItemInSlot(const FARTInventoryItemSlotReference& Reference)
{
	if (!IsValidItemSlot(Reference))
	{
		return nullptr;
	}

	FARTInventoryItemSlot& ItemSlot = GetItemSlot(Reference);
	BagInventory.MarkItemDirty(ItemSlot);
	return ItemSlot.ItemStack;
}

void UARTInventoryComponent::OnRep_BagInventory()
{
	//Check all of the item stacks and ensure they have a proper outer.  
	for (int32 i = 0; i < BagInventory.Slots.Num(); i++)
	{
		FARTInventoryItemSlot& Slot = BagInventory.Slots[i];
		if (IsValid(Slot.ItemStack) && Slot.ItemStack->GetOuter() != GetOwner())
		{
			Slot.ItemStack->Rename(nullptr, GetOwner());
		}
	}
	OnInventoryUpdate.Broadcast(this);
}

bool UARTInventoryComponent::IsValidItemSlot(const FARTInventoryItemSlotReference& Slot)
{	
	//Check to see if we contain this reference
	for (const FARTInventoryItemSlotReference& Ref : AllReferences)
	{
		if (Slot == Ref)
		{
			return true;
		}
	}

	return false;
}

FARTInventoryItemSlot& UARTInventoryComponent::GetItemSlot(const FARTInventoryItemSlotReference& RefSlot)
{
	check(IsValidItemSlot(RefSlot));

	for (FARTInventoryItemSlot& SlotSlot : BagInventory.Slots)
	{		
		if (RefSlot == SlotSlot)
		{
			BagInventory.MarkItemDirty(SlotSlot);
			return SlotSlot;
		}
	}

	//We'll never hit this, since we check IsValidItemSlot
	return BagInventory.Slots[0];
}

int32 UARTInventoryComponent::GetInventorySize()
{	
	return BagInventory.Slots.Num();
}


TArray<FARTInventoryItemSlotReference> UARTInventoryComponent::GetAllSlotReferences()
{
	return AllReferences;
}

void UARTInventoryComponent::PopulateSlotReferenceArray(TArray<FARTInventoryItemSlotReference>& RefArray)
{
	for (int i = 0; i < BagInventory.Slots.Num(); i++)
	{
		FARTInventoryItemSlotReference SlotRef(BagInventory.Slots[i], this);
		RefArray.Add(SlotRef);
	}
}

#pragma region Debugging

struct FARTInventoryDebugTargetInfo
{
	FARTInventoryDebugTargetInfo()
	{

	}

	TWeakObjectPtr<UWorld> TargetWorld;
	TWeakObjectPtr<UARTInventoryComponent> LastDebugTarget;
};

TArray<FARTInventoryDebugTargetInfo> InventoryDebugInfoList;

FARTInventoryDebugTargetInfo* GetDebugTargetInfo(UWorld* World)
{
	FARTInventoryDebugTargetInfo* TargetInfo = nullptr;
	for (FARTInventoryDebugTargetInfo& Info : InventoryDebugInfoList)
	{
		if (Info.TargetWorld.Get() == World)
		{
			TargetInfo = &Info;
			break;
		}
	}
	if (TargetInfo == nullptr)
	{
		TargetInfo = &InventoryDebugInfoList[InventoryDebugInfoList.AddDefaulted()];
		TargetInfo->TargetWorld = World;
	}

	return TargetInfo;
}

UARTInventoryComponent* GetDebugTarget(FARTInventoryDebugTargetInfo* TargetInfo)
{
	//Return the Target if we have one
	if (UARTInventoryComponent* Inv = TargetInfo->LastDebugTarget.Get())
	{
		return Inv;
	}

	//Find one
	for (TObjectIterator<UARTInventoryComponent> It; It; ++It)
	{
		if (UARTInventoryComponent* Inv = *It)
		{
			if (Inv->GetWorld() == TargetInfo->TargetWorld.Get() && MakeWeakObjectPtr(Inv).Get())
			{
				TargetInfo->LastDebugTarget = Inv;

				//Default to local player
				if (APawn* Pawn = Cast<APawn>(Inv->GetOwner()))
				{
					if (Pawn->IsLocallyControlled())
					{
						break;
					}
				}
			}
		}
	}
	return TargetInfo->LastDebugTarget.Get();
}



//FARTOnItemSlotUpdate& UARTInventoryComponent::GetItemSlotUpdateDelegate(const FARTInventoryItemSlotReference& ItemSlotRef)
//{
//	return ItemSlotUpdates.FindOrAdd(ItemSlotRef);
//}

class UAbilitySystemComponent* UARTInventoryComponent::GetOwnerAbilitySystem()
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

void UARTInventoryComponent::PostInventoryUpdate()
{
	AllReferences.Empty(AllReferences.Num() + 1);
	PopulateSlotReferenceArray(AllReferences); 
	
	OnInventoryUpdate.Broadcast(this);
}

bool UARTInventoryComponent::Query_GetAllSlots(const FARTItemQuery& Query, TArray<FARTInventoryItemSlotReference>& OutSlotRefs)
{
	for (FARTInventoryItemSlot& ItemSlot : BagInventory.Slots)
	{
		if (Query.MatchesSlot(ItemSlot))
		{
			OutSlotRefs.Add(FARTInventoryItemSlotReference(ItemSlot, this));
		}
	}
	return OutSlotRefs.Num() > 0;
}

FARTInventoryItemSlotReference UARTInventoryComponent::Query_GetFirstSlot(const FARTItemQuery& Query)
{
	TArray<FARTInventoryItemSlotReference> OutSlotRefs;
	
	if (!Query_GetAllSlots(Query, OutSlotRefs))
	{
		UE_LOG(LogInventory, Warning, TEXT("Tried to query for %s but didn't find it"), *Query.SlotTypeQuery.GetDescription())
		return FARTInventoryItemSlotReference();
	}

	return OutSlotRefs[0];
}



void UARTInventoryComponent::Query_GetAllItems(const FARTItemQuery& Query, TArray<UARTItemStack*>& OutItems)
{
	for (FARTInventoryItemSlot& ItemSlot : BagInventory.Slots)
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

static TAutoConsoleVariable<int32> DetailedItemStackInfo(TEXT("ARTInventory.DebugDetailedItemInfo"), 0, TEXT(""), ECVF_Default);

void UARTInventoryComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("Inventory")) )
	{
		UWorld* World = HUD->GetWorld();
		FARTInventoryDebugTargetInfo* TargetInfo = GetDebugTargetInfo(World);

		if (UARTInventoryComponent* Inv = GetDebugTarget(TargetInfo))
		{
			TArray<FName> LocalDisplayNames;
			LocalDisplayNames.Add(TEXT("CInventory"));
			FDebugDisplayInfo LocalDisplayInfo(LocalDisplayNames, TArray<FName>());

			Inv->DisplayDebug(Canvas, LocalDisplayInfo, YL, YPos);
		}
	}
}

void UARTInventoryComponent::DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	if (DebugDisplay.IsDisplayOn(TEXT("CInventory")))
	{
		FInventoryComponentDebugInfo DebugInfo;

		DebugInfo.bPrintToLog = false;
		DebugInfo.Canvas = Canvas;
		DebugInfo.XPos = 0.f;
		DebugInfo.YPos = YPos;
		DebugInfo.OriginalX = 0.f;
		DebugInfo.OriginalY = YPos;
		DebugInfo.MaxY = Canvas->ClipY - 150.f; // Give some padding for any non-columnizing debug output following this output
		DebugInfo.NewColumnYPadding = 30.f;

		Debug_Internal(DebugInfo);

		YPos = DebugInfo.YPos;
		YL = DebugInfo.YL;
	}
}

void UARTInventoryComponent::PrintDebug()
{

}

void UARTInventoryComponent::Debug_Internal(struct FInventoryComponentDebugInfo& Info)
{
	bool DetailedDisplay = DetailedItemStackInfo.GetValueOnGameThread() > 0;

	{	//Debug Title

		FString DebugTitle("Inventory ");
		
		DebugTitle += FString::Printf(TEXT("for actor %s "), *GetOwner()->GetName());
		if (GetOwnerRole() == ROLE_AutonomousProxy) DebugTitle += TEXT("(Local Player)");
		else if (GetOwnerRole() == ROLE_SimulatedProxy) DebugTitle += TEXT("(Simulated)");
		else if (GetOwnerRole() == ROLE_Authority) DebugTitle += TEXT("(Authority)");
		
		if (Info.Canvas)
		{
			Info.Canvas->SetDrawColor(FColor::White);
			Info.Canvas->DrawText(GEngine->GetLargeFont(), DebugTitle, Info.XPos + 4.f, 10.f, 1.5f, 1.5f);
		}
		else
		{
			DebugLine(Info, DebugTitle, 0.f, 0.f);
		}  	
	}

	

	//Draw the bag inventory
	{
		DebugLine(Info, FString::Printf(TEXT("Bag Inventory (Slots: %d)"), GetInventorySize()), 0.0f, 0.0f);
		ForEachItemSlot_ReadOnly([&Info, this, DetailedDisplay](const FARTInventoryItemSlot& InventorySlot) {
			TArray<FString> DebugStrings;
			InventorySlot.ToDebugStrings(DebugStrings, DetailedDisplay);

			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Yellow);
			}
			DebugLine(Info, DebugStrings[0], 4.0f, 0.0f);

			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Cyan);
			}
			for (int i = 1; i < DebugStrings.Num(); i++)
			{
				DebugLine(Info, DebugStrings[i], 12.0f, 0.0f);
			}
		});
	}
}

void UARTInventoryComponent::AccumulateScreenPos(FInventoryComponentDebugInfo& Info)
{
	const float ColumnWidth = Info.Canvas ? Info.Canvas->ClipX * 0.4f : 0.f;

	float NewY = Info.YPos + Info.YL;
	if (NewY > Info.MaxY)
	{
		// Need new column, reset Y to original height
		NewY = Info.NewColumnYPadding;
		Info.XPos += ColumnWidth;
	}
	Info.YPos = NewY;
}



void UARTInventoryComponent::DebugLine(struct FInventoryComponentDebugInfo& Info, FString Str, float XOffset, float YOffset)
{
	if (Info.Canvas)
	{
		Info.YL = Info.Canvas->DrawText(GEngine->GetTinyFont(), Str, Info.XPos + XOffset, Info.YPos);
		AccumulateScreenPos(Info);
	}

	if (Info.bPrintToLog)
	{
		FString LogStr;
		for (int32 i = 0; i < (int32)XOffset; ++i)
		{
			LogStr += TEXT(" ");
		}
		LogStr += Str;
		UE_LOG(LogInventory, Warning, TEXT("%s"), *LogStr);
	}

	if (Info.Accumulate)
	{
		FString LogStr;
		for (int32 i = 0; i < (int32)XOffset; ++i)
		{
			LogStr += TEXT(" ");
		}
		LogStr += Str;
		Info.Strings.Add(Str);
	}
}

namespace InventoryDebug
{
	FDelegateHandle DebugHandle = AHUD::OnShowDebugInfo.AddStatic(&UARTInventoryComponent::OnShowDebugInfo);
}



#pragma endregion