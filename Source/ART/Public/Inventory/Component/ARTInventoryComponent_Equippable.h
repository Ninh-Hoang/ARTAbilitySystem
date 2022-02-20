// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Component/ARTInventoryComponent_Storage.h"
#include "ARTInventoryComponent_Equippable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FARTInventoryItemInfoEntry
{
	GENERATED_BODY()
public:
	FARTInventoryItemInfoEntry()
	{

	}
	FARTInventoryItemInfoEntry(const FARTItemSlotReference& Ref)
		: ItemSlotRef(Ref)
	{

	}
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FARTItemSlotReference ItemSlotRef;
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FARTEquippedItemInfo EquippedItemInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTInvOnItemEquippedDelegate, class UARTInventoryComponent*, Inventory, const FARTItemSlotReference&, ItemSlotRef, UARTItemStack*, ItemStack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTInvOnAttributeSetCreated, class UARTInventoryComponent_Equippable*, Inventory, class UAttributeSet*, AttributeSet, UARTItemStack*, AttributeSource);


UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class ART_API UARTInventoryComponent_Equippable : public UARTInventoryComponent_Storage
{
	GENERATED_BODY()
public:
	UARTInventoryComponent_Equippable(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



	UFUNCTION()
	virtual void OnItemEquipped(class UARTInventoryComponent* Inventory, const FARTItemSlotReference& ItemSlotRef, UARTItemStack* ItemStack, UARTItemStack* PreviousItemStack);

	virtual bool IsEquippedItemSlot(const FARTItemSlotReference& ItemSlotRef);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities", Replicated, meta = (AllowPrivateAccess = "true"))
	TArray<FARTInventoryItemInfoEntry> EquippedItemAbilityInfos;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnItemEquippedDelegate OnEquippedItem;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnItemEquippedDelegate OnUnEquippedItem;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnAttributeSetCreated OnAttributeSetCreated;
	
protected:
	virtual bool MakeItemEquipped_Internal(const FARTItemSlotReference& ItemSlot);
	virtual bool MakeItemEquipped_Internal(const FARTItemSlotReference& ItemSlot, UARTItemStack* ItemStack);
	virtual bool MakeItemUnequipped_Internal(const FARTItemSlotReference& ItemSlot);
	virtual bool MakeItemUnequipped_Internal(const FARTItemSlotReference& ItemSlot, UARTItemStack* ItemStack);

	virtual bool ApplyAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto, UARTItemStack* AbilitySource);
	virtual bool ClearAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto);

	virtual void ApplyMods(UARTItemStack* ItemStack, const FARTItemSlotReference& ItemSlot);
	virtual void RemoveMods(UARTItemStack* ItemStack, const FARTItemSlotReference& ItemSlot);

private: 	
	virtual void Debug_Internal(struct FInventoryComponentDebugInfo& Info) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllEquippableSlots"))
	bool Query_GetAllEquippableSlots(TArray<FARTItemSlotReference>& OutSlotRefs);
};
