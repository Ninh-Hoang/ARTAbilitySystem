// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Generator/ARTItemGenerator.h"
#include "ARTItemGenerator_Unique.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTItemGenerator_Unique : public UARTItemGenerator
{
	
	GENERATED_BODY()
public:
	UARTItemGenerator_Unique(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Definition")
	TSubclassOf<UARTItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	UARTItemRarity* ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FText ItemDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Definition")
	TArray<UARTItemGenerator*> SubItemGenerators;

	virtual UARTItemStack* GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context) override;
};
