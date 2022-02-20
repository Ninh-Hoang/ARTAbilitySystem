// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Component/ARTInventoryComponent_Equippable.h"
#include "ARTInventoryComponent_Active.generated.h"

/**
 * 
 */

class UARTItemStack;
class UARTInventoryComponent_Active;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FARTOnItemActivationDelegate, UARTInventoryComponent_Active*, InventoryComponent, UARTItemStack*, ItemStack);


UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class ART_API UARTInventoryComponent_Active : public UARTInventoryComponent_Equippable
{
	GENERATED_BODY()
public:
	UARTInventoryComponent_Active(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Called when the game stARTs
	virtual void BeginPlay() override;

	virtual void OnItemEquipped(class UARTInventoryComponent* Inventory, const FARTItemSlotReference& ItemSlotRef, UARTItemStack* ItemStack, UARTItemStack* PreviousItemStack) override;
	   
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual FARTItemSlotReference GetActiveItemSlot();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual UARTItemStack* GetActiveItemStack();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual FARTItemSlotReference GetActiveItemSlotInSlot(int32 InActiveItemSlot);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual UARTItemStack* GetActiveItemStackInSlot(int32 InActiveItemSlot);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual bool IsValidActiveItemSlot(int32 InActiveItemSlot);

	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ActiveItemSlot, Category = "Inventory")
	int32 ActiveItemSlot;
	UFUNCTION()
	virtual void OnRep_ActiveItemSlot(int32 PreviousItemSlot);


	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_PendingItemSlot, Category = "Inventory")
	int32 PendingItemSlot;
	UFUNCTION()
	virtual void OnRep_PendingItemSlot(int32 PreviousItemSlot);

	//Returns the next Active Item Slot, regardless if that slot has a valid item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetNextActiveItemSlot() const;

	//Returns the previous Active Item Slot, regardless if that slot has a valid item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetPreviousActiveItemSlot() const;

	//Returns the next Active Item Slot, but only if it has a valid item.  
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetNextValidActiveItemSlot();

	//Returns the previous Active Item Slot, but only if it has a valid item.  
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual int32 GetPreviousValidActiveItemSlot();

	virtual int32 GetNextItemSlotFrom(int32 InActiveItemSlot) const;
	virtual int32 GetPreviousItemSlotFrom(int32 InActiveItemSlot) const;

	virtual void MakeItemInactive();
	virtual void MakeItemActive(int32 NewActiveItemSlot);
	virtual void MakeItemActive(const FARTItemSlotReference& ItemSlotRef);

	virtual bool IsActiveItemSlot(const FARTItemSlotReference& ItemSlotRef) const;
	
	virtual int32 GetActiveItemIndexBySlotRef(const FARTItemSlotReference& ItemSlotRef);
	virtual int32 GetActiveItemIndexByTag(FGameplayTag Tag);

	virtual void SwitchToPendingItemSlot();

	UFUNCTION()
	virtual void UpdateActiveItemSlots(UARTInventoryComponent* InventoryComp);

	bool bSwitchingWeapons;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SwapActiveItems(int32 NewItemSlot);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities", Replicated)
	TArray< FARTInventoryItemInfoEntry> ActiveItemAbilityInfos;

	UPROPERTY(BlueprintAssignable)
	FARTOnItemActivationDelegate OnItemActive;
	UPROPERTY(BlueprintAssignable)
	FARTOnItemActivationDelegate OnItemInactive;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetIndexForActiveItemSlotTagQuery(const FGameplayTagQuery& TagQuery);

protected:
	virtual bool MakeItemInactive_Internal(const FARTItemSlotReference& ItemSlot);
	virtual bool MakeItemInactive_Internal(const FARTItemSlotReference& ItemSlot, UARTItemStack* ItemStack);
	virtual bool MakeItemActive_Internal(const FARTItemSlotReference& ItemSlot);
	virtual bool MakeItemActive_Internal(const FARTItemSlotReference& ItemSlot, UARTItemStack* ItemStack);


	TArray<FARTItemSlotReference> CachedActiveItemSlots;
};
