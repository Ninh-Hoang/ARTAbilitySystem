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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UARTItemStack> ItemStackClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Default")
	FGameplayTagContainer OwnedTags;
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AppendTags(OwnedTags);
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "UI", Meta = (AssetBundles = "UI"))
	UARTItemUIData* UIData;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Generator", Meta = (AutoCreateRefTerm =
		"Context"))
	UARTItemStack* GenerateItemStack(const FARTItemGeneratorContext& Context);
	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context);
		
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	TSubclassOf<UARTItemGenerator> StaticGenerator;

	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context) override;
};
