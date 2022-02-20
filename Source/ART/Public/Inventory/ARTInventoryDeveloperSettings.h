// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ARTInventoryDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(config=Game, defaultconfig)
class ART_API UARTInventoryDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UARTInventoryDeveloperSettings(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", config)
	TSubclassOf<class UARTItemStack> DefaultItemStackClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", config)
	TSubclassOf<class AARTItemStackWorldObject> ItemStackWorldObjectClass;
};
