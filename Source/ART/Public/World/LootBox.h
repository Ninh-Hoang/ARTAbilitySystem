// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootBox.generated.h"

class UItem;

UCLASS()
class ART_API ALootBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALootBox();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Loot Box")
	TSubclassOf<AActor> Item;

	virtual void BeginPlay() override;

	void SpawnItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnItem();
};
