// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Generator/ARTItemGenerator.h"
#include "ARTItemGenerator_SimpleRandom.generated.h"

/**
 * 
 */
class UARTItemRarity;
class UARTItemStack;
class UARTItemStack_Mod;

USTRUCT(BlueprintType)
struct FARTItemGenerator_RaritySelector
{
	GENERATED_USTRUCT_BODY()
public:
	FARTItemGenerator_RaritySelector()
	{
		NumberOfMods = 1;
		NumberOfModifiers = 1;
		Weight = 1;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	FGameplayTagQuery RarityTagQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 NumberOfMods;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 NumberOfModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 Weight; 
};

USTRUCT(BlueprintType)
struct FARTItemGenerator_DynamicModifier
{
	GENERATED_USTRUCT_BODY()
public:

	FARTItemGenerator_DynamicModifier()
	{
		MinValue = 0;
		MaxValue = 1;
		ModifierOp = EGameplayModOp::Multiplicitive;
		Weight = 1;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		FGameplayAttribute Attribute;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		TEnumAsByte<EGameplayModOp::Type> ModifierOp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float MinValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float MaxValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayModifier)
	FGameplayTagQuery SlotApplicationRequirement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 Weight;
};

USTRUCT(BlueprintType)
struct FARTItemGenerator_ModSelector
{
	GENERATED_USTRUCT_BODY()
public:

	FARTItemGenerator_ModSelector()
	{
		Weight = 1;
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	FGameplayTagQuery ModTagQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	int32 Weight;
};

/**
 * 
 */
UCLASS(NotEditInlineNew, Abstract)
class ART_API UARTItemGenerator_SimpleRandom : public UARTItemGenerator
{
	GENERATED_BODY()
public:

	UARTItemGenerator_SimpleRandom(const FObjectInitializer& ObjectInitializer);
	

	//Tag Query for picking up any item definition and generating this item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	FGameplayTagQuery ItemTagQuery;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TArray< FARTItemGenerator_RaritySelector> RarityTable;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	bool bRequireUniqueModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TArray<FARTItemGenerator_DynamicModifier> WeightedPossibleModifiers;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mods")
	bool bRequireUniqueMods;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mods")
	TArray<FARTItemGenerator_ModSelector> WeightedPossibleMods;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, Category = "Mods")
	UARTItemGenerator* DynamicModItemGenerator;
																   	
		
	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context) override;

protected:
	
		  
};


USTRUCT(BlueprintType)
struct FARTItemGenerator_GeneratorSelector
{
	GENERATED_USTRUCT_BODY()
public:
	FARTItemGenerator_GeneratorSelector()
	{
		ItemGen = nullptr;
		Weight = 1;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", Instanced)
	UARTItemGenerator* ItemGen;
		   			 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	int32 Weight;
};

UCLASS(EditInlineNew)
class ART_API UARTItemGenerator_SelectGenerator : public UARTItemGenerator
{
	GENERATED_BODY()
public:
	UARTItemGenerator_SelectGenerator(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TArray<FARTItemGenerator_GeneratorSelector> WeightedGenerators;


	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context) override;

};
