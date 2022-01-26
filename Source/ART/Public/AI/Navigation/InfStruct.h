// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct ART_API FInfNode
{
	FInfNode() :
		GraphLocation(FIntVector(0)),
		NodeLocation(FVector(0)){}

private:
	FIntVector GraphLocation;
	FVector NodeLocation;
	
	TArray<FIntVector> Neighbor;	
	TArray<uint32> RegionTileID;

public:
	void SetGraphLocation(const FIntVector& NewGraphLocation) {GraphLocation = NewGraphLocation; }
	void AddNeighbor(const FIntVector& NeighborNode) { Neighbor.Add(NeighborNode); }
	void AddRegionTileID(uint32 NewRegionTileID) {RegionTileID.AddUnique(NewRegionTileID); }
	void ResetRegionTileID() {RegionTileID.Reset(); }
	void SetNodeLocation(const FVector& InLocation) {NodeLocation = InLocation; }; 
	
	void RemoveNeighbor(const FIntVector& Value) {Neighbor.Remove(Value);}
	void RemoveNeighbor(int32 Index) {Neighbor.RemoveAt(Index);}
	
	const FIntVector& GetGraphLocation() const { return GraphLocation; }
	const TArray<FIntVector>& GetNeighbor() const { return Neighbor; }
	const TArray<uint32>& GetRegionTileID() const { return RegionTileID; }
	const FVector& GetNodeLocation() const {return NodeLocation; }
	
	bool operator==(const FInfNode* Other) const { return GraphLocation == Other->GraphLocation; }
	bool operator==(const FInfNode& Other) const { return GraphLocation == Other.GraphLocation; }

	bool operator!=(const FInfNode* Other) const { return !(GraphLocation == Other->GraphLocation); }
	bool operator!=(const FInfNode& Other) const { return !(GraphLocation == Other.GraphLocation); }
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