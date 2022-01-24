// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfNode.h"
#include "GameFramework/Actor.h"
#include "InfNodeGraphInterface.h"
#include "InfNodeGraph.generated.h"

UENUM()
enum class EDebugMenu : uint8
{
	NODES,
	NODES_NEIGHBOR,
};

UCLASS()
class ART_API AInfNodeGraph : public AActor, public IInfNodeGraphInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInfNodeGraph();

	// ƒm[ƒhƒOƒ‰ƒt‚Ì¶¬.
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Generator")
	bool bGenerateNodeGraph;
	// ƒm[ƒhƒOƒ‰ƒt‚ÌÁ‹Ž
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Generator")
	bool bClearNodeGraph;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	EDebugMenu DrawDebugType;
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bDrawNodeGraph;
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bClearDrawDebug;

private:
	TMap<FIntVector, FInfNode*> NodeGraph;
	UPROPERTY()
	const class AInfNavMesh* NavMeshCache;

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void GenerateGraph();
	void SpawnAndConnectingNodes();
	void ConnectingNodeIslands();
	void ClearGraph();

	//inf node graph interface
public:
	virtual FInfNode* FindNearestNode(const FVector& FeetLocation) const override;
	virtual const TMap<FIntVector, FInfNode*>& GetNodeGraphData()const override;
	virtual const FInfNode* GetNode(const FIntVector& Key) const override;

private:
	int FindNavmeshTilesContainsLocation(const FVector& FeetLocation) const;
	FIntVector FindNearestNodeKey(int TargetTileIdx, const FVector& FeetLocation) const;

private:

	void DrawDebugNodeGraph(bool bDrawConnectingNeighbor = false) const;
};
