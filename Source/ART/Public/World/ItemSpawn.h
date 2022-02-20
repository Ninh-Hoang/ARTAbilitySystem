// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "Engine/DataTable.h"
#include "ItemSpawn.generated.h"

/**
 * 
 */
class UItem;
class UDataTable;
class APickup;

USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = 0.001, ClampMax = 1.))
	float Probability;
};

UCLASS()
class ART_API AItemSpawn : public ATargetPoint
{
	GENERATED_BODY()

public:
	AItemSpawn();

	UPROPERTY(EditAnywhere, Category = "Loot")
	UDataTable* LootTable;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<APickup> PickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	FIntPoint RespawnRange;

protected:

	FTimerHandle TimerHandle_RepsawnItem;

	UPROPERTY()
	TArray<AActor*> SpawnedPickups;

	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnItem();

	UFUNCTION()
	void OnItemTaken(AActor* DestroyedActor);
};
