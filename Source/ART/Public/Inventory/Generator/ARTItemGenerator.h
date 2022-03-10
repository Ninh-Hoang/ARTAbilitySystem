// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagAssetInterface.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "ARTItemGenerator.generated.h"

/**
 * 
 */

class UARTItemStack;
class UARTItemUIData;
class UARTItemDefinition;
class UARTItemRarity;

UCLASS(Blueprintable, Abstract, EditInlineNew)
class ART_API UARTItemGenerator : public UObject, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:
	UARTItemGenerator(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	TSubclassOf<UARTItemStack> ItemStackClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Default")
	FGameplayTagContainer OwnedTags;
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AppendTags(OwnedTags);
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "UI", Meta = (AssetBundles = "UI"))
	UARTItemUIData* UIData;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Generator")
	UARTItemStack* GenerateItemStack(const FARTItemGeneratorContext& Context = FARTItemGeneratorContext());
	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context = FARTItemGeneratorContext());
		
protected:
	virtual UARTItemStack* CreateNewItemStack(UARTItemDefinition* ItemDefinition, UARTItemRarity* ItemRarity);
	virtual void PostCreateNewItemStack(UARTItemStack* ItemStack);
};

UCLASS()
class ART_API UARTItemGenerator_Static : public UARTItemGenerator
{
	GENERATED_BODY()
public:
	UARTItemGenerator_Static(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator")
	TSubclassOf<UARTItemGenerator> StaticGenerator;

	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context) override;
};
