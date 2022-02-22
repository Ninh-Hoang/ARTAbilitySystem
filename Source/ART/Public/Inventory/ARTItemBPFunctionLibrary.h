// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTInventoryItemTypes.h"
#include "ARTItemStack.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ARTItemBPFunctionLibrary.generated.h"

/**
 * 
 */

class UARTItemDefinition;
class UARTItemRarity;
class UARTItemGenerator;
class UARTItemStack;
class UARTItemStack_SlotContainer;

UCLASS()
class ART_API UARTItemBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UARTItemBPFunctionLibrary();

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static FString GetAttributeString(FGameplayAttribute Attribute);

	// Asset Management
	template <typename T>
	static bool QueryMatchingAssets(FGameplayTagQuery Query, TArray<T*>& OutResults);

	template <typename T>
	static bool QueryMatchingAssetClasses(FGameplayTagQuery Query, TArray<TSubclassOf<T>>& OutResults);

	UFUNCTION(BlueprintCallable, Category = "ART|Assets")
	static bool QueryMatchingItemDefinitions(FGameplayTagQuery Query, TArray<TSubclassOf<UARTItemDefinition>>& OutItemDefinitions);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	static bool QueryMatchingItemRarities(FGameplayTagQuery Query, TArray<UARTItemRarity*>& OutItemRarities);
		
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory")
	static bool QueryMatchingItemGenerators(FGameplayTagQuery Query, TArray<UARTItemGenerator*>& OutItemGenerators);



	//Inventory
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static UARTInventoryComponent* GetInventoryComponent(AActor* Actor, bool bSearchComponents = false);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static class UARTInventoryComponent_Storage* GetStorageInventoryComponent(AActor* Actor, bool bSearchComponents = false);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static class UARTInventoryComponent_Equippable* GetEquippableInventoryComponent(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static class UARTInventoryComponent_Active* GetActiveInventoryComponent(AActor* Actor, bool bSearchComponents = false);
	
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|Debug", meta = (WorldContext = "WorldContextObject"))
	static class AARTItemStackWorldObject* SpawnWorldItem(UObject* WorldContextObject,  UARTItemStack* ItemStack, const FTransform& Transform);
	
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static class UARTItemUIData_ItemDefinition* GetUIDataFromItemDefinition(TSubclassOf<UARTItemDefinition> ItemDefinition);

	//ItemStack Operation
	UFUNCTION(BlueprintPure, Category= "ART|Inventory")
	static bool TwoItemCanStack(UARTItemStack* TargetStack,UARTItemStack* SourceStack);

	UFUNCTION(BlueprintCallable, Category= "ART|Inventory")
	static bool RemoveItemInSlot(const FARTItemSlotRef& ItemSlot);

	UFUNCTION(BlueprintCallable, Category= "ART|Inventory")
	static bool SwapItemSlot(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot);

	UFUNCTION(BlueprintPure, Category= "ART|Inventory")
	static bool CanAcceptSlotItem_AssumeEmptySlot(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot);

	UFUNCTION(BlueprintPure, Category= "ART|Inventory")
	static bool CanAcceptItem_AssumeEmptySlot(UARTItemStack* Item, const FARTItemSlotRef& ToSlot);

	static bool DoesItemContainSlot(UARTItemStack* Item, const FARTItemSlotRef& Slot);
	
	////INVENTORY SLOTS
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static bool IsValidItemSlot(const FARTItemSlotRef& ItemSlotRef);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static UARTItemStack* GetItemFromSlot(const FARTItemSlotRef& ItemSlotRef);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal ARTInventoryItemSlotReference", CompactNodeTitle = "==", Keywords = "== equal"), Category = "ART|Inventory")
	static bool EqualEqual_FARTItemSlotRef(const FARTItemSlotRef& ItemSlotRef, const FARTItemSlotRef& OtherItemSlotRef);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal ARTInventoryItemSlotReference", CompactNodeTitle = "!=", Keywords = "!= equal"), Category = "ART|Inventory")
	static bool NotEqual_FARTItemSlotRef(const FARTItemSlotRef& ItemSlotRef, const FARTItemSlotRef& OtherItemSlotRef);
	
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static UARTInventoryComponent* GetInventoryFromSlot(const FARTItemSlotRef& ItemSlotRef);

	/////INVENTORY QUERY
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static bool IsValidInventoryQuery(const FARTSlotQueryHandle& Query);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory", Meta = (DisplayName =
	"Make Slot Query Handle GameplayTagQuery", AutoCreateRefTerm =
	"SlotQuery, ItemQuery"))
	static FARTSlotQueryHandle MakeSlotQueryHandle_GameplayTagQuery(
		const FGameplayTagQuery& SlotQuery,
		const FGameplayTagQuery& ItemQuery);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory", Meta = (DisplayName =
	"Make Slot Query Handle Find Slot With Item", AutoCreateRefTerm =
	"ItemRequiredTags, ItemBlockedTags"))
	static FARTSlotQueryHandle MakeSlotQueryHandle_SlotWithItem(
		const TSubclassOf<UARTItemDefinition> ItemDefinition,
		const TEnumAsByte<EItemStackCount::Type> StackCount,
		const FGameplayTagContainer& ItemRequiredTags,
		const FGameplayTagContainer& ItemBlockedTags);

	UFUNCTION(BlueprintPure, Category = "Ability|Inventory", Meta = (DisplayName =
	"Make Slot Query Handle Find Slot Can Accept Item"))	
	static FARTSlotQueryHandle MakeSlotQueryHandle_SlotCanAcceptItem(
		const TEnumAsByte< EItemExistence::Type> ItemExist,
		UARTItemStack* ContextItemStack);

	//Attribute

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static float GetActiveAttributeFromItemSlot(const FARTItemSlotRef& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static float GetEquippedAttributeFromItemSlot(const FARTItemSlotRef& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute);

	
	static void CopyAttributeSet(UAttributeSet* Src, UAttributeSet* Destination);

	static bool ASCHasAttributeSet(UAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass);
	static bool ASCAddInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet);
	static bool ASCRemoveInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet);

	
	
};
