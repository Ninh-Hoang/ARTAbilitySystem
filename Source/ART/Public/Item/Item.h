// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Engine/DataAsset.h>
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

/**
 * 
 */
class UStaticMesh;
class UTexture2D;
class UInventoryComponent;
class AARTSurvivor;
class UItemTooltip;

UENUM()
enum class EItemRarity : uint8
{
	IR_Common UMETA(DisplayName = "Common"),
	IR_Uncommon UMETA(DisplayName = "Uncommon"),
	IR_Rare UMETA(DisplayName = "Rare"),
	IR_VeryRare UMETA(DisplayName = "Very Rare"),
	IR_SuperRare UMETA(DisplayName = "Super Rare")
};


UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class ART_API UItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItem();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable)
	)
	int32 MaxStackSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<UItemTooltip> ItemTooltip;

	UPROPERTY(ReplicatedUsing=OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition =
		bStackable))
	int32 Quantity;

	UPROPERTY()
	UInventoryComponent* OwningInventory;

	UPROPERTY()
	int32 RepKey;

	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

	//Gameplay Ability System
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ARTPickup")
	TArray<TSubclassOf<class UARTGameplayAbility>> AbilityClasses;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ARTPickup")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool ShouldShowInInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetStackWeight() const { return Weight * Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetQuantity() const { return Quantity; }

	virtual void Use(AARTSurvivor* Character);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(AARTSurvivor* Character);

	virtual void AddedToInventory(UInventoryComponent* Inventory);

	UFUNCTION()
	void OnRep_Quantity();
	
	void MarkDirtyForReplication();
};
