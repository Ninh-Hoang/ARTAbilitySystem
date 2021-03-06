// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InfGraphInterface.h"
#include "InfType.h"
#include "InfGraph.generated.h"

UENUM()
enum class EDebugMenu : uint8
{
	NODES,
	NODES_NEIGHBOR,
	SINGLE_NODE,
};

UCLASS()
class ART_API AInfGraph : public AActor, public IInfGraphInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInfGraph();

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Generator")
	bool bGenerateNodeGraph;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Generator")
	bool RuntimeGeneration;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Generator")
	bool bClearNodeGraph;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	EDebugMenu DrawDebugType;
	
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bDrawNodeGraph;
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bClearDrawDebug;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	FIntVector NodeToDraw;

private:
	FInfMap NodeGraph;
	UPROPERTY()
	class AInfNavMesh* NavMeshCache;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void GenerateGraph();
	void SpawnAndConnectingNodes();
	void ConnectingNodeIslands();
	void ClearGraph();

	UFUNCTION()
	void OnNeedUpdateGraph(const TArray<uint32>& ChangedTiles);

	//inf node graph interface
public:
	virtual FInfNode* FindNearestNode(const FVector& FeetLocation) const override;
	virtual const FInfMap* GetNodeGraphData() const override;
	virtual const FInfNode* GetNode(const FIntVector& Key) const override;
	const FVector GetNodeLocation(const FIntVector& Key) const;

private:
	int FindNavmeshTilesContainsLocation(const FVector& FeetLocation) const;
	FIntVector FindNearestNodeKey(int TargetTileIdx, const FVector& FeetLocation) const;

private:
	
	void DrawDebugNodeGraph() const;
	bool InDebugRange(const FVector& Location) const;
};
