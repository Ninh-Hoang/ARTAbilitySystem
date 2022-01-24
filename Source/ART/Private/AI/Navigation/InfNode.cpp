// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfNode.h"

FInfNode::FInfNode()
{
	ID = INDEX_NONE;
	RegionTileID = INDEX_NONE;
	GraphLocation = FIntVector(0);
}

void FInfNode::SetID(int NewID)
{
	ID = NewID;
}

void FInfNode::SetGraphLocation(FIntVector NewGraphLocation)
{
	GraphLocation = NewGraphLocation;
}

void FInfNode::AddNeighbor(FInfNode* NeighborNode)
{
	Neighbor.Add(NeighborNode);
}

void FInfNode::SetRegionTileID(int NewRegionTileID)
{
	RegionTileID = NewRegionTileID;
}

void FInfNode::SetSpawnSegment(FVector Begin, FVector End)
{
	SpawnSegmentBegin = Begin;
	SpawnSegmentEnd = End;
}

void FInfNode::SetNodeLocation(FVector InLocation)
{
	NodeLocation = InLocation;
}
