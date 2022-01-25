// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct ART_API FInfNode
{
	FInfNode() :
		ID(INDEX_NONE),
		GraphLocation(FIntVector(0)),
		RegionTileID(INDEX_NONE){}

private:
	uint_fast32_t ID;
	FIntVector GraphLocation;
	TArray<FIntVector> Neighbor;	
	uint_fast32_t RegionTileID;
	
	FVector SpawnSegmentBegin;
	FVector SpawnSegmentEnd;

	//TODO: Debug only?
	FVector NodeLocation;

public:
	void SetID(int NewID) { ID = NewID; }
	void SetGraphLocation(const FIntVector& NewGraphLocation) {GraphLocation = NewGraphLocation; }
	void AddNeighbor(const FIntVector& NeighborNode) { Neighbor.Add(NeighborNode); }
	void SetRegionTileID(uint_fast32_t NewRegionTileID) {RegionTileID = NewRegionTileID; };
	void SetNodeLocation(const FVector& InLocation) {NodeLocation = InLocation; }; 
	void SetSpawnSegment(const FVector& Begin, const FVector& End)
	{
		SpawnSegmentBegin = Begin;
		SpawnSegmentEnd = End;
	}

	const uint_fast32_t GetID() const { return ID; }
	const FIntVector& GetGraphLocation() const { return GraphLocation; }
	const TArray<FIntVector>& GetNeighbor() const { return Neighbor; }
	const uint_fast32_t GetRegionTileID() const { return RegionTileID; }
	const FVector& GetNodeLocation() const {return NodeLocation; }
	const FVector& GetSpawnSegmentBegin() const { return SpawnSegmentBegin; }
	const FVector& GetSpawnSegmentEnd() const { return SpawnSegmentEnd; }
	
	bool operator==(const FInfNode* Other) const { return ID == Other->ID; }
	bool operator==(const FInfNode& Other) const { return ID == Other.ID; }

	bool operator!=(const FInfNode* Other) const { return !(ID == Other->ID); }
	bool operator!=(const FInfNode& Other) const { return !(ID == Other.ID); }
};

struct ART_API FInfMap
{
	TMap<FIntVector, FInfNode> NodeMap;

	void Reset()
	{
		NodeMap.Empty();
	}

	int32 GetSize() {
		int Size = 0;
		Size += NodeMap.Num() * sizeof(FInfNode);
		Size += NodeMap.Num() * sizeof(FIntVector);
		return Size;
	}
};