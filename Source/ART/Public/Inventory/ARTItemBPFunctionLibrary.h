// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTInventoryItemTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ARTItemBPFunctionLibrary.generated.h"

/**
 * 
 */

class UARTItemDefinition;
class UARTItemRarity;
class UARTItemGenerator;

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
	
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static float GetActiveAttributeFromItemSlot(const FARTInventoryItemSlotReference& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static float GetEquippedAttributeFromItemSlot(const FARTInventoryItemSlotReference& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute);


	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|Debug", meta = (WorldContext = "WorldContextObject"))
	static class AARTItemStackWorldObject* SpawnWorldItem(UObject* WorldContextObject,  UARTItemStack* ItemStack, const FTransform& Transform);
		
	
																														
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static class UARTItemUIData_ItemDefinition* GetUIDataFromItemDefinition(TSubclassOf<UARTItemDefinition> ItemDefinition);


	////INVENTORY SLOTS
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static bool IsValidItemSlotRef(const FARTInventoryItemSlotReference& ItemSlotRef);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static UARTItemStack* GetItemFromSlot(const FARTInventoryItemSlotReference& ItemSlotRef);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal ARTInventoryItemSlotReference", CompactNodeTitle = "==", Keywords = "== equal"), Category = "ART|Inventory")
	static bool EqualEqual_FARTInventoryItemSlotReference(const FARTInventoryItemSlotReference& ItemSlotRef, const FARTInventoryItemSlotReference& OtherItemSlotRef);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static UARTInventoryComponent* GetInventoryFromSlot(const FARTInventoryItemSlotReference& ItemSlotRef);

	/////INVENTORY QUERY
	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static bool IsValidInventoryQuery(const FARTItemQuery& Query);	

	UFUNCTION(BlueprintPure, Category = "ART|Inventory")
	static FGameplayTagQuery MakeGameplayTagQuery_AnyTag(const FGameplayTagContainer& TagContainer);

	static void CopyAttributeSet(UAttributeSet* Src, UAttributeSet* Destination);

	static bool ASCHasAttributeSet(UAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass);
	static bool ASCAddInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet);
	static bool ASCRemoveInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet);
	
};
