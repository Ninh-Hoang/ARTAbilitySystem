// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventorySet.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FItemApplicationInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UItem> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
};

UCLASS()
class ART_API UInventorySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FItemApplicationInfo> StartInventory;

	void InitInventory(class UInventoryComponent* InventoryComponent) const;
};
