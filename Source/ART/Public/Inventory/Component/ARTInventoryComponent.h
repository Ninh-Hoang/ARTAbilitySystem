// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "Engine/NetSerialization.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Components/ActorComponent.h"
#include "ARTInventoryComponent.generated.h"

class UARTInventoryAttributeSet;
class UARTItemStack;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTOnInventoryUpdate, class UARTInventoryComponent*, Inventory);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FARTOnItemSlotUpdate, class UARTInventoryComponent*, Inventory, const FARTItemSlotRef&, ItemSlotRef, UARTItemStack*, ItemStack, UARTItemStack*, PreviousItemStack );

UCLASS()
class ART_API UARTInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	friend struct FARTItemSlot;

	// Sets default values for this component's properties
	UARTInventoryComponent(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitProperties() override;
	virtual void InitializeComponent() override;

protected:
	//Creates a new slot with given tags
	virtual void CreateInventorySlot(const FGameplayTagContainer& SlotTags, const FARTItemSlotFilterHandle& Filter);

	virtual void RemoveInventorySlot(const FARTItemSlotRef& Slot);

public:		
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	//Returns true if the item slot reference is valid.  Override this if adding additional item slots
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual bool IsValidItemSlot(const FARTItemSlotRef& Slot);

	//Returns true if the item slot reference is valid.  Override this if adding additional item slots
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual bool IsValidItemSlotRef(const FARTItemSlotRef& Slot);
	
	//Returns a reference to the item slot.  Call IsValidItemSlot before this to ensure you get a valid item slot
	virtual FARTItemSlot& GetItemSlot(const FARTItemSlotRef& RefSlot);
		
	//Returns true if the item has been add to this inventory.  False if the item can't fit. 
	UFUNCTION(BlueprintCallable, Category="ART|Inventory")
	virtual bool LootItem(UARTItemStack* Item);

	//Places the item into the slot.  Returns false if hte item cannot be put there.
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool PlaceItemIntoSlot(UARTItemStack* Item, const FARTItemSlotRef& ItemSlot);

	//Returns true if the item was in the inventory and is now removed, false if the item could not be removed for whatever reason
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool RemoveItemFromInventory(const FARTItemSlotRef& ItemSlot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool RemoveAllItemsFromInventory(TArray<UARTItemStack*>& OutItemsRemoved);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool SwapItemSlots(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool AcceptsItem(UARTItemStack* Item, const FARTItemSlotRef& Slot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool AcceptsItem_AssumeEmptySlot(UARTItemStack* Item, const FARTItemSlotRef& Slot);

	virtual UARTItemStack* GetItemInSlot(const FARTItemSlotRef& Reference);
	  
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FARTItemSlotRef SwapFromSlot;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FARTItemSlotRef SwapToSlot;

	UFUNCTION(Category = "ART|Inventory")
	virtual void OnRep_BagInventory();
	   
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual int32 GetInventorySize();	    											  

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual TArray<FARTItemSlotRef> GetAllSlotReferences();

	void PopulateSlotReferenceArray(TArray<FARTItemSlotRef>& RefArray);


	UPROPERTY(BlueprintAssignable, Category = "ART|Inventory")
	FARTOnInventoryUpdate OnInventoryUpdate;

	UPROPERTY(BlueprintAssignable, Category = "ART|Inventory")
	FARTOnItemSlotUpdate OnItemSlotChange;

	//TODO: item slot delegate implementation
	//TMap<FARTItemSlotRef, FARTOnItemSlotUpdate> ItemSlotUpdates;

	//FARTOnItemSlotUpdate& GetItemSlotUpdateDelegate(const FARTItemSlotRef& ItemSlotRef);
	 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Layout")
	TArray< FARTItemSlotDefinition> CustomInventorySlots;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Input")
	FARTItemSlotRef PendingItemDrop;

	virtual class UAbilitySystemComponent* GetOwnerAbilitySystem();


protected:
	UPROPERTY(BlueprintReadOnly,Replicated)
	FARTItemSlotArray BagInventory;
	
private:
	TArray<FARTItemSlotRef> AllReferences;

	int32 IdCounter;

	void PostInventoryUpdate();

	//Inventory Searching
public:
	UFUNCTION(BlueprintCallable, Category="Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAll"))
	bool Query_GetAllSlots(const FARTSlotQueryHandle& Query, TArray<FARTItemSlotRef>& OutSlotRefs);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetFirst"))
	FARTItemSlotRef Query_GetFirstSlot(const FARTSlotQueryHandle& Query);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllItems"))
	void Query_GetAllItems(const FARTSlotQueryHandle& Query, TArray<UARTItemStack*>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetFirstItem"))
	UARTItemStack* Query_GetFirstItem(const FARTSlotQueryHandle& Query);
	
	//Iterate through each item slot.
	//Lambda looks like this: [](const FARTItemSlot& Slot) { }
	template<typename PRED>
	void ForEachItemSlot_ReadOnly(PRED Predicate) const
	{
		ForEachItemSlot_ReadOnly(FARTSlotQueryHandle(), Predicate);
	}

	//Iterate through each item slot, matching a slot query.
	//Lambda looks like this: [](const FARTItemSlot& Slot) { }
	template<typename PRED>
	void ForEachItemSlot_ReadOnly(const FARTSlotQueryHandle& Query, PRED Predicate) const
	{
		for (const FARTItemSlot& ItemSlot : BagInventory.Slots)
		{
			if (Query.MatchesSlot(ItemSlot)) 
			{
				Predicate(ItemSlot);				
			}
		}
	}

	//Iterate through each item slot.
	//Note: This is a writable version.  It will mark any item slot touched for replication, regardless of changes
	//Use the _ReadOnly version if you are just trying to read the slots
	//Lambda looks like this: [](FARTItemSlot& Slot) { }
	template<typename PRED>
	void ForEachItemSlot_Mutable(PRED Predicate)
	{
		ForEachItemSlot_Mutable(FARTSlotQueryHandle(), Predicate);
	}

	//Iterate through each item slot, matching a slot query.
	//Note: This is a writable version.  It will mark any item slot touched for replication, regardless of changes
	//Use the _ReadOnly version if you are just trying to read the slots
	//Lambda looks like this: [](FARTItemSlot& Slot) { }
	template<typename PRED>
	void ForEachItemSlot_Mutable(const FARTSlotQueryHandle& Query, PRED Predicate)
	{
		for (FARTItemSlot& ItemSlot : BagInventory.Slots)
		{
			if (Query.MatchesSlot(ItemSlot))
			{
				Predicate(ItemSlot);
				BagInventory.MarkItemDirty(ItemSlot);
			}
		}
	}

	//Debugging Section
public:
	struct FInventoryComponentDebugInfo
	{
		FInventoryComponentDebugInfo()
		{
			FMemory::Memzero(*this);
		}

		class UCanvas* Canvas;

		bool bPrintToLog;
									  		
		float XPos;
		float YPos;
		float OriginalX;
		float OriginalY;
		float MaxY;
		float NewColumnYPadding;
		float YL;

		bool Accumulate;
		TArray<FString>	Strings;

		int32 GameFlags; // arbitrary flags for games to set/read in Debug_Internal
	};

	static void OnShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos);
	virtual void PrintDebug();

	void AccumulateScreenPos(FInventoryComponentDebugInfo& Info);
	virtual void Debug_Internal(struct FInventoryComponentDebugInfo& Info);
	void DebugLine(struct FInventoryComponentDebugInfo& Info, FString Str, float XOffset, float YOffset);	
};
