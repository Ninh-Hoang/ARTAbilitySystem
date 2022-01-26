// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InfMapFunctionLibrary.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMapOperationResult
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FIntVector, float> ResultMap;

	TPair<FIntVector, float> HighestPair;
};


UCLASS()
class ART_API UInfMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static class AInfNavMesh* GetInfNavMesh(const UObject* WorldContext);
	static void DestroyAllButFirstSpawnActor(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);
	static FColor ConvertInfluenceValueToColor(float Value);


	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	static FMapOperationResult InitializeWorkingMap(const TMap<FIntVector, float>& NewMap);

	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	static FMapOperationResult AddTargetMap(const FMapOperationResult& MapA, const TMap<FIntVector, float>& MapB, float Weight = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	static FMapOperationResult MultTargetMap(const FMapOperationResult& MapA, const TMap<FIntVector, float>& MapB, float Weight = 1.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FMapOperationResult InvertTargetMap(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FMapOperationResult NormalizeTargetMap(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FIntVector GetHighestLocation(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map", meta = (DisplayName = "SelectNearbyHighestLocation"))
	static FIntVector SelectNearbyHighestLocation(const FMapOperationResult& TargetMap, const FVector& CurrentLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map", meta = (DisplayName = "SelectRandomLocationAtLeast"))
	static FIntVector SelectLocationOfLeastInfluenceValue(const FMapOperationResult& TargetMap, float Percent);
};
