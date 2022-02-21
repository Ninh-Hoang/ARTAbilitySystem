// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Component/ARTInventoryComponent.h"
#include "ARTInventoryComponent_Storage.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class ART_API UARTInventoryComponent_Storage : public UARTInventoryComponent
{
	GENERATED_BODY()
public:
	UARTInventoryComponent_Storage(const FObjectInitializer& ObjectInitializer);

public:
	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void UpdateStorageInventorySlots();

	//Number of slots this inventory contains
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	int32 StorageSlotCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	FGameplayAttribute InventorySizeAttribute;
protected:
	
	virtual void OnInventorySizeAttributeChange(const FOnAttributeChangeData& ChangeData);

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory | Item Queries", meta = (ScriptName = "ItemQuery_GetAllStorageSlots"))
	bool Query_GetAllStorageSlots(TArray<FARTItemSlotRef>& OutSlotRefs);
	
};
