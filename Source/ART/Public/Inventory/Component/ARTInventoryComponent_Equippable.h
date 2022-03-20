// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Component/ARTInventoryComponent_Storage.h"
#include "Inventory/Mod/ARTItemDefinition_Mod.h"
#include "ARTInventoryComponent_Equippable.generated.h"

/**
 * 
 */
class UARTItemStack_Mod;

USTRUCT(BlueprintType)
struct FARTItemModHandlePair : public FFastArraySerializerItem
{
	GENERATED_BODY()
 
	FARTItemModHandlePair(){}
 
	FARTItemModHandlePair(UARTItemStack_Mod* InModStack, FARTModPropertyHandle InHandle)
		: ModStack(InModStack), Handle(InHandle) {}
 
private:
	friend struct FARTItemModHandleContainer;

	UPROPERTY()
	UARTItemStack_Mod* ModStack;
	
	UPROPERTY()
	FARTModPropertyHandle Handle;
};

USTRUCT(BlueprintType)
struct FARTItemModHandleContainer : public FFastArraySerializer
{
	GENERATED_BODY()
 
	FARTItemModHandleContainer() { }
 
public:
	void AddItemModHandlePair(FARTItemModHandlePair& Pair);
 	
 	void RemoveItemModHandlePair(UARTItemStack_Mod* ItemStack);
	
	FARTModPropertyHandle& GetModPropertyHandle(const UARTItemStack_Mod* ItemStack) const;
	
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
     
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FARTItemModHandlePair, FARTItemModHandleContainer>(ItemModHandlePairs, DeltaParms, *this);
	}
	
	
     
 
private:
	// List of gameplay tag stacks. Use the accelerated TagCountMap for checking tag count, etc.
	UPROPERTY()
	TArray<FARTItemModHandlePair> ItemModHandlePairs;
};
 
template<>
struct TStructOpsTypeTraits<FARTItemModHandleContainer> : public TStructOpsTypeTraitsBase2<FARTItemModHandleContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct FARTInventoryItemInfoEntry
{
	GENERATED_BODY()
public:
	FARTInventoryItemInfoEntry(){}
	FARTInventoryItemInfoEntry(const FARTItemSlotRef& Ref)
		: ItemSlotRef(Ref){}
	
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FARTItemSlotRef ItemSlotRef;
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FARTEquippedItemInfo EquippedItemInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTInvOnItemEquippedDelegate, class UARTInventoryComponent*, Inventory, const FARTItemSlotRef&, ItemSlotRef, UARTItemStack*, ItemStack);
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
	virtual void OnItemEquipped(class UARTInventoryComponent* Inventory, const FARTItemSlotRef& ItemSlotRef, UARTItemStack* ItemStack, UARTItemStack* PreviousItemStack);

	virtual bool IsEquippedItemSlot(const FARTItemSlotRef& ItemSlotRef);

	UPROPERTY(BlueprintReadOnly, Category = "Abilities", Replicated, meta = (AllowPrivateAccess = "true"))
	TArray<FARTInventoryItemInfoEntry> EquippedItemAbilityInfos;

	UPROPERTY(BlueprintReadOnly,Replicated)
	FARTItemModHandleContainer ItemModHandleContainer;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnItemEquippedDelegate OnEquippedItem;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnItemEquippedDelegate OnUnEquippedItem;

	UPROPERTY(BlueprintAssignable)
	FARTInvOnAttributeSetCreated OnAttributeSetCreated;
	
protected:
	virtual bool MakeItemEquipped_Internal(const FARTItemSlotRef& ItemSlot);
	virtual bool MakeItemEquipped_Internal(const FARTItemSlotRef& ItemSlot, UARTItemStack* ItemStack);
	virtual bool MakeItemUnequipped_Internal(const FARTItemSlotRef& ItemSlot);
	virtual bool MakeItemUnequipped_Internal(const FARTItemSlotRef& ItemSlot, UARTItemStack* ItemStack);

	virtual bool ApplyAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto, UARTItemStack* AbilitySource);
	virtual bool ClearAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto);

	virtual void ApplyMods(UARTItemStack* ItemStack, const FARTItemSlotRef& ItemSlot);
	virtual void RemoveMods(UARTItemStack* ItemStack, const FARTItemSlotRef& ItemSlot);

private: 	
	virtual void Debug_Internal(struct FInventoryComponentDebugInfo& Info) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllEquippableSlots"))
	bool Query_GetAllEquippableSlots(TArray<FARTItemSlotRef>& OutSlotRefs);
};
