// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfNode.generated.h"

USTRUCT(BlueprintType)
struct ART_API FInfNode
{
	GENERATED_BODY()
	
	FInfNode();

private:
	int ID;
	FIntVector GraphLocation;
	TArray<FInfNode*> Neighbor;	
	int RegionTileID;
	//TODO: Debug only?
	FVector  NodeLocation;

	FVector SpawnSegmentBegin;
	FVector SpawnSegmentEnd;

public:
	void SetID(int NewID);
	void SetGraphLocation(FIntVector NewGraphLocation);
	void AddNeighbor(FInfNode* NeighborNode);
	void SetRegionTileID(int NewRegionTileID);
	void SetNodeLocation(FVector InLocation);
	void SetSpawnSegment(FVector Begin, FVector End);

	int GetID() const { return ID; }
	const FIntVector GetGraphLocation() const { return GraphLocation; }
	const TArray<FInfNode*>& GetNeighbor() const { return Neighbor; }
	const int GetRegionTileID() const { return RegionTileID; }
	const FVector GetNodeLocation() const {return NodeLocation; }
	const FVector GetSpawnSegmentBegin() const { return SpawnSegmentBegin; }
	const FVector GetSpawnSegmentEnd() const { return SpawnSegmentEnd; }

	bool operator==(FInfNode* Other) const { return ID == Other->ID; }
	bool operator==(const FInfNode* Other) const { return ID == Other->ID; }

	bool operator==(FInfNode& Other) const { return ID == Other.ID; }
	bool operator==(const FInfNode& Other) const { return ID == Other.ID; }
};
