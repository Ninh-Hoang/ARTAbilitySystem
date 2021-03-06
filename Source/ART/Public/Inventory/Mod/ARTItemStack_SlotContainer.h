// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "Inventory/ARTItemStack.h"
#include "ARTItemStack_SlotContainer.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTOnContainerUpdate, class UARTItemStack_SlotContainer*, ItemStack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FARTOnContainerSlotUpdate, class UARTItemStack_SlotContainer*, ParentStack, const FARTItemSlotRef&, ItemSlotRef, UARTItemStack*, ChildStack, UARTItemStack*, PreviousItemStack);


UCLASS(Blueprintable, BlueprintType)
class ART_API UARTItemStack_SlotContainer : public UARTItemStack
{
	GENERATED_BODY()
	
public:
	UARTItemStack_SlotContainer(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeContainer(TArray<FARTItemSlotDefinition>& CustomInventorySlots);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	UPROPERTY(BlueprintAssignable)
	FARTOnContainerUpdate OnContainerUpdate;

	UPROPERTY(BlueprintAssignable)
	FARTOnContainerSlotUpdate OnContainerSlotUpdate;

	void PostContainerUpdate();
	void PopulateSlotReferenceArray(TArray<FARTItemSlotRef>& RefArray);
	
	//Returns true if the item slot reference is valid.  Override this if adding additional item slots
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual bool IsValidItemSlot(const FARTItemSlotRef& Slot);
	
	//Returns a reference to the item slot.  Call IsValidItemSlot before this to ensure you get a valid item slot
	virtual FARTItemSlot& GetItemSlot(const FARTItemSlotRef& RefSlot);

	//Returns true if the item has been add to this inventory.  False if the item can't fit. 
	UFUNCTION(BlueprintCallable, Category="ART|Inventory")
	virtual bool LootItem(UARTItemStack* Item);
	
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool PlaceItemIntoSlot(UARTItemStack* Item, const FARTItemSlotRef& ItemSlot);
	
	//Returns true if the item was in the inventory and is now removed, false if the item could not be removed for whatever reason
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool RemoveItemFromContainer(const FARTItemSlotRef& ItemSlot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool RemoveAllItemsFromContainer(TArray<UARTItemStack*>& OutItemsRemoved);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool SwapItemSlots(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool AcceptsItem(UARTItemStack* Item, const FARTItemSlotRef& Slot);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	virtual bool AcceptsItem_AssumeEmptySlot(UARTItemStack* Item, const FARTItemSlotRef& Slot);

	virtual UARTItemStack* GetItemInSlot(const FARTItemSlotRef& Reference);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual int32 GetContainerSize();

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual int32 GetItemCount();

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	virtual TArray<FARTItemSlotRef> GetAllSlotReferences();

	//CONTAINER QUERYING

	UFUNCTION(BlueprintCallable, Category="Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllSlots"))
	bool Query_GetAllSlots(const FARTSlotQueryHandle& Query, TArray<FARTItemSlotRef>& OutSlotRefs);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetFirstSlot"))
	FARTItemSlotRef Query_GetFirstSlot(const FARTSlotQueryHandle& Query);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllItems"))
	void Query_GetAllItems(const FARTSlotQueryHandle& Query, TArray<UARTItemStack*>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetFirstItem"))
	UARTItemStack* Query_GetFirstItem(const FARTSlotQueryHandle& Query);

	//replication code
	UFUNCTION()
	virtual void OnRep_ItemContainer();
protected:
	virtual void CreateContainerSlot(const FGameplayTagContainer& SlotTags, const FARTItemSlotFilterHandle& Filter);

	virtual void RemoveInventorySlot(const FARTItemSlotRef& Slot);

protected:
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_ItemContainer)
	FARTItemSlotArray ItemContainer;
private:
	UPROPERTY()
	TArray<FARTItemSlotRef> AllReferences;

	UPROPERTY()
	TArray<FARTItemSlotRef> Items;
	
	int32 IdCounter;
};
