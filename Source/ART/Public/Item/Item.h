// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

/**
 * 
 */
class UStaticMesh;
class UItemTooltip;
class UGameplayAbility;
class UGameplayEffect;
class AARTCharacterBase;

UENUM()
enum class EItemRarity : uint8
{
	IR_Common UMETA(DisplayName = "Common"),
	IR_Uncommon UMETA(DisplayName = "Uncommon"),
	IR_Rare UMETA(DisplayName = "Rare"),
	IR_VeryRare UMETA(DisplayName = "Very Rare"),
	IR_SuperRare UMETA(DisplayName = "Super Rare")
};

USTRUCT(BlueprintType)
struct FItem_AbilityData
{
	GENERATED_BODY()

	TSoftClassPtr<UGameplayAbility> AbilityClass;
	int32 Level = 1;
};

USTRUCT(BlueprintType)
struct FItem_GameplayEffectData
{
	GENERATED_BODY()

	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;
	int32 Level = 1;
};


UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class ART_API UItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item)
	FPrimaryAssetType ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UIGame"))
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (MultiLine = true), meta = (AssetBundles = "UI"))
	FText ItemDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UI"))
	FText UseActionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UIGame"))
	EItemRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UI"))
	bool bStackable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UI"))
	bool AllowMultipleStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (ClampMin = 2, EditCondition = bStackable), meta = (AssetBundles = "UI"))
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (ClampMin = 0.0), meta = (AssetBundles = "UIGame"))
	float Weight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UI"))
	FSlateBrush Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "Game"))
	TSoftObjectPtr<UStaticMesh> PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "UI"))
	TSoftClassPtr<UItemTooltip> ItemTooltip;

	//Gameplay Ability System
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Item, meta = (AssetBundles = "Ability"))
	TArray<FItem_AbilityData> GrantedAbilities;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Item, meta = (AssetBundles = "Ability"))
	TArray<FItem_GameplayEffectData> GrantedEffects;

	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual bool ShouldReplicateItemData(const struct FItemData& ItemData) const { return true;}
	
	UFUNCTION(BlueprintPure, Category = Item)
	virtual bool ShouldShowInInventory() const;

	virtual void Use(AARTCharacterBase* Character);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(AARTCharacterBase* Character);

	/** Returns the logical name, equivalent to the primary asset id */
	UFUNCTION(BlueprintCallable, Category = Item)
	FString GetIdentifierString() const;

	/** Overridden to use saved type */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
