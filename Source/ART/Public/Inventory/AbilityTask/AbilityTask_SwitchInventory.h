// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTInvAbilityTask_SimpleTarget.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "AbilityTask_SwitchInventory.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_ItemSwitch : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY()
	FARTInventoryItemSlotReference FromSlot;
	UPROPERTY()
	FARTInventoryItemSlotReference ToSlot;

	// -------------------------------------

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_ItemSwitch::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return TEXT("FGameplayAbilityTargetData_ItemSwitch");
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ItemSwitch> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ItemSwitch>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FARTSwitchItemsRecieved, const FARTInventoryItemSlotReference&, FromSlot, const FARTInventoryItemSlotReference&, ToSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FARTItemSwitchCancelled);

UCLASS()
class ART_API UAbilityTask_SwitchInventory : public UAbilityTask_SimpleInvTarget
{
	GENERATED_BODY()
public:
	UAbilityTask_SwitchInventory(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAbilityTask_SwitchInventory* SwitchInventorySlots(UGameplayAbility* OwningAbility, const FARTInventoryItemSlotReference& FromSlot, const FARTInventoryItemSlotReference& ToSlot);
	
	virtual FGameplayAbilityTargetDataHandle GenerateTargetHandle() override;
	virtual void HandleTargetData(const FGameplayAbilityTargetDataHandle& Data) override;
	virtual void HandleCancelled() override;

	UPROPERTY(BlueprintAssignable)	
	FARTSwitchItemsRecieved OnSlotsReceived;

	UPROPERTY(BlueprintAssignable)
	FARTItemSwitchCancelled OnSlotsCancelled;

protected:

	FARTInventoryItemSlotReference FromSlot;
	FARTInventoryItemSlotReference ToSlot;
};
