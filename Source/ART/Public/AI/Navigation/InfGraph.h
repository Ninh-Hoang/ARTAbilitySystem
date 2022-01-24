// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InfGraphInterface.h"
#include "InfStruct.h"
#include "InfGraph.generated.h"

UENUM()
enum class EDebugMenu : uint8
{
	NODES,
	NODES_NEIGHBOR,
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
	bool bClearNodeGraph;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	EDebugMenu DrawDebugType;
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bDrawNodeGraph;
	UPROPERTY(EditAnywhere, Category = "NodeGraph | Debug")
	bool bClearDrawDebug;

private:
	FInfMap NodeGraph;
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
	virtual FInfNode* FindNearestNode(const FVector& FeetLocation) override;
	virtual FInfMap* GetNodeGraphData() override;
	virtual FInfNode* GetNode(const FIntVector& Key) override;
	const FVector GetNodeLocation(const FIntVector& Key) const;

private:
	int FindNavmeshTilesContainsLocation(const FVector& FeetLocation) const;
	FIntVector FindNearestNodeKey(int TargetTileIdx, const FVector& FeetLocation) const;

private:
	
	void DrawDebugNodeGraph(bool bDrawConnectingNeighbor = false) const;
	bool InDebugRange(FVector Location) const;
};
