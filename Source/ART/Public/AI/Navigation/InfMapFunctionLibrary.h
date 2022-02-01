// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
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

	TPair<FIntVector, float> LowestPair;
};

UENUM()
enum class EOperationType : uint8
{
	Add,
	Mult,
	Invert,
	Normalize
};

USTRUCT(BlueprintType)
struct FOperationData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation)
	EOperationType OperationType;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation, meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	FGameplayTag MapTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation, meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	float GatherDistance = 0.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation, meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	FGameplayTagContainer BehaviourTags;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation, meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	FGameplayTagContainer RequiredTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = IMapOperation, meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	FGameplayTagContainer BlockedTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "IMapOperation", meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	bool bIgnoreSelf = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "IMapOperation", meta = (ClampMax = 10.00, ClampMin = -10.00), meta = (EditCondition = "OperationType == EOperationType::Add || OperationType == EOperationType::Mult"))
	float Weight = 0.f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "IMapOperation", meta = (EditCondition = "OperationType == EOperationType::Add"))
	bool Inclusive = true;
};

USTRUCT(BlueprintType)
struct ART_API FInfQueryData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = IMapOperation)
	TArray<FOperationData> OperationConstructData;

	UPROPERTY(EditAnywhere, Category = IMapOperation)
	class UInfPropagator* Propagator;
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
	static FMapOperationResult& AddTargetMap(const FMapOperationResult& MapA, const TMap<FIntVector, float>& MapB, float Weight = 1.f, bool Inclusive = true);

	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	static FMapOperationResult& MultTargetMap(const FMapOperationResult& MapA, const TMap<FIntVector, float>& MapB, float Weight = 1.f, bool Inclusive = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FMapOperationResult& InvertTargetMap(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FMapOperationResult& NormalizeTargetMap(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FIntVector GetHighestLocation(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FIntVector GetLowestLocation(const FMapOperationResult& TargetMap);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map", meta = (DisplayName = "SelectNearbyHighestLocation"))
	static FIntVector SelectNearbyHighestLocation(const FMapOperationResult& TargetMap, const FVector& CurrentLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map", meta = (DisplayName = "SelectRandomLocationAtLeast"))
	static FIntVector SelectLocationOfLeastInfluenceValue(const FMapOperationResult& TargetMap, float Percent);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FMapOperationResult GetInfluenceMapFromQuery(const FInfQueryData& QueryData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	static FInfQueryData MakeInfluenceQueryData(const TArray<FOperationData>& InfluenceOperation, class UInfPropagator* InPropagator);
};
