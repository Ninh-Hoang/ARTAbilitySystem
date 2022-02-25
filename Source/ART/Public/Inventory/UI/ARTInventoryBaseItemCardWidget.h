// Copyright 2017-2020 Puny Human, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "ARTInventoryBaseItemCardWidget.generated.h"

class UARTItemStack;

/**
 * 
 */
UCLASS(Abstract)
class ART_API UARTInventoryBaseItemCardWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UARTInventoryBaseItemCardWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "ARTInventory|UI")
	virtual UARTItemStack* GetItemStack() const;

	UFUNCTION(BlueprintCallable, Category="ARTInventory|UI")
	virtual void SetItemStack(UARTItemStack* InItemStack);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnItemStackSet(UARTItemStack* NewItemStack);

	UFUNCTION(BlueprintPure, Category = "ARTInventory|UI")
	virtual FARTItemSlotRef& GetItemSlotRef();
	
	UFUNCTION(BlueprintCallable, Category="ARTInventory|UI")
	void SetItemSlotRef(FARTItemSlotRef& InSlotRef);
	
protected:

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="", BlueprintSetter=SetItemStack, BlueprintGetter=GetItemStack, Meta=(ExposeOnSpawn = true))
	UARTItemStack* ItemStack;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="", BlueprintSetter=SetItemSlotRef, BlueprintGetter=GetItemSlotRef, Meta=(ExposeOnSpawn = true))
	FARTItemSlotRef ItemSlotRef;
};
