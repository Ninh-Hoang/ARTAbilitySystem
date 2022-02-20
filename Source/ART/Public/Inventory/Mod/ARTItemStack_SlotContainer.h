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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FARTOnContainerSlotUpdate, class UARTItemStack_SlotContainer*, ParentStack, const FARTItemSlotReference&, ItemSlotRef, UARTItemStack*, ChildStack, UARTItemStack*, PreviousItemStack);


UCLASS(Blueprintable, BlueprintType)
class ART_API UARTItemStack_SlotContainer : public UARTItemStack
{
	GENERATED_BODY()

public:
	UARTItemStack_SlotContainer(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeItemStack();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;


protected:
	virtual void CreateContainerSlot(const FGameplayTagContainer& SlotTags, const FARTItemSlotFilterHandle& Filter);

	virtual void RemoveInventorySlot(const FARTItemSlotReference& Slot);
	
private:
	UPROPERTY(Replicated)
	FARTItemSlotArray ItemContainer;
	
	TArray<FARTItemSlotReference> AllReferences;

	int32 IdCounter;
	
};
