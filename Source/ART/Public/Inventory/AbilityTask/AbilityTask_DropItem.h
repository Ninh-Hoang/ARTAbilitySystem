// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTInvAbilityTask_SimpleTarget.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "AbilityTask_DropItem.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct ART_API FGameplayAbilityTargetData_DropItem : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY()
	FARTItemSlotReference FromSlot;
	

	// -------------------------------------

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_DropItem::StaticStruct();
	}

	virtual FString ToString() const override
	{
		return TEXT("FGameplayAbilityTargetData_DropItem");
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_DropItem> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_DropItem>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTDropItemRecieved, const FARTItemSlotReference&, FromSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FARTDropItemCancelled);

UCLASS()
class ART_API UAbilityTask_DropItem : public UAbilityTask_SimpleInvTarget
{
	GENERATED_BODY()
	
public:
	UAbilityTask_DropItem(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAbilityTask_DropItem* DropItemFromInventory(UGameplayAbility* OwningAbility, const FARTItemSlotReference& FromSlot);


	virtual FGameplayAbilityTargetDataHandle GenerateTargetHandle() override;
	virtual void HandleTargetData(const FGameplayAbilityTargetDataHandle& Data) override;
	virtual void HandleCancelled() override;

	UPROPERTY(BlueprintAssignable)
	FARTDropItemRecieved OnDataRecieved;
	UPROPERTY(BlueprintAssignable)
	FARTDropItemCancelled OnDataCancelled;

protected:
	FARTItemSlotReference SlotReference;
};
