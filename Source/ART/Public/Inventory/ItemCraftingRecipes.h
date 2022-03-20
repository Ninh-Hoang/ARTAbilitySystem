// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ItemCraftingRecipes.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct ART_API FCraftingIngredient
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer IngredientTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=1))
	int32 IngredientAmount;
};

USTRUCT(BlueprintType)
struct ART_API FCraftingOutputItem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UARTItemDefinition> OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=1))
	int32 OutputAmount;
};

USTRUCT(BlueprintType)
struct  ART_API FItemCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CraftingTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* CraftingIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCraftingIngredient> Ingredients;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCraftingOutputItem> OutputItems;
};