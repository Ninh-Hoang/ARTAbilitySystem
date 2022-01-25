// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfGraph.h"
#include "chrono"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AI/Navigation/InfNavMesh.h"
#include "Math/Box.h"

namespace FNavMeshRenderingHelpers
{
	inline uint8 GetBit(int32 v, uint8 bit)
	{
		return (v & (1 << bit)) >> bit;
	}

	FColor GetClusterColor(int32 Idx)
	{
		uint8 r = 1 + GetBit(Idx, 1) + GetBit(Idx, 3) * 2;
		uint8 g = 1 + GetBit(Idx, 2) + GetBit(Idx, 4) * 2;
		uint8 b = 1 + GetBit(Idx, 0) + GetBit(Idx, 5) * 2;
		return FColor(r * 63, g * 63, b * 63, 255);
	}
}

// Sets default values
AInfGraph::AInfGraph()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bGenerateNodeGraph = false;
	RuntimeGeneration = true;
	bClearNodeGraph = false;
	DrawDebugType = EDebugMenu::NODES_NEIGHBOR;
	bDrawNodeGraph = false;
	bClearDrawDebug = false;
}

void AInfGraph::OnConstruction(const FTransform& Transform)
{
	UInfMapFunctionLibrary::DestroyAllButFirstSpawnActor(this, StaticClass());
}

void AInfGraph::BeginPlay()
{
	Super::BeginPlay();
	if(RuntimeGeneration)
	{
		ClearGraph();
		GenerateGraph();
	}
}

void AInfGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL)
		                     ? PropertyChangedEvent.Property->GetFName()
		                     : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfGraph, bGenerateNodeGraph))
	{
		bGenerateNodeGraph = false;
		GenerateGraph();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfGraph, bClearNodeGraph))
	{
		bClearNodeGraph = false;
		ClearGraph();
		FlushPersistentDebugLines(GetWorld());
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfGraph, bDrawNodeGraph))
	{
		bDrawNodeGraph = false;
		FlushPersistentDebugLines(GetWorld());
		DrawDebugNodeGraph(DrawDebugType == EDebugMenu::NODES_NEIGHBOR);
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfGraph, bClearDrawDebug))
	{
		bClearDrawDebug = false;
		FlushPersistentDebugLines(GetWorld());
	}
}

void AInfGraph::GenerateGraph()
{
	NavMeshCache = UInfMapFunctionLibrary::GetInfNavMesh(this);
	if (NavMeshCache == nullptr)
	{
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "FGraph::GenerateGraph() : Add the element with [Influence Recast Navmesh] specified in Nav Data Class to Supported Agents."
		       ));
		return;
	}
#if WITH_EDITOR
	const auto StartTime = std::chrono::high_resolution_clock::now();
#endif
	
	// generating empty infmap
	SpawnAndConnectingNodes();
	ConnectingNodeIslands();

#if WITH_EDITOR
	const float Duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count() / 1000.0f;
	int32 NumBytes = NodeGraph.GetSize();
	
	UE_LOG(LogTemp, Display, TEXT("Generation Time : %f seconds"), Duration);
	UE_LOG(LogTemp, Display, TEXT("Total Graph btyes : %i"), NumBytes);
#endif
}

void AInfGraph::SpawnAndConnectingNodes()
{
	int NewID = 0, SkipCount = 0;
	const int NavMeshTileCount = NavMeshCache->GetNavMeshTilesCount();
	for (int TileSetIdx = 0; TileSetIdx < NavMeshTileCount; TileSetIdx++)
	{
		TArray<FNavPoly> Polys;

		// skip if no poly
		if (!NavMeshCache->GetPolysInTile(TileSetIdx, Polys))
		{
			SkipCount++;
			continue;
		}

		// iterate poly in tile
		for (const FNavPoly& Poly : Polys)
		{
			TArray<FVector> PolyVerts;

			//skip if this poly has no vertex
			if (!NavMeshCache->GetPolyVerts(Poly.Ref, PolyVerts))
			{
				SkipCount++;
				continue;
			}

			TArray<FIntVector> SpawnNodes;

			//spawn a node for each poly vert
			SpawnNodes.Reserve(PolyVerts.Num());

			// iterate verts
			for (int VertIdx = 0; VertIdx < PolyVerts.Num(); VertIdx++)
			{
				//TODO: refine this 
				FVector Middle = (PolyVerts[VertIdx] + PolyVerts[(VertIdx + 1) % PolyVerts.Num()]) * 0.5f;
				FIntVector IntMiddle = FIntVector(Middle);

				FInfNode* Node;
				if(!NodeGraph.NodeMap.Contains(IntMiddle))
				{
					FInfNode TempNode = FInfNode();
					Node = &TempNode;
					Node->SetID(NewID++);
					Node->SetRegionTileID(TileSetIdx);
					Node->SetGraphLocation(IntMiddle);
					Node->SetSpawnSegment(PolyVerts[VertIdx], PolyVerts[(VertIdx + 1) % PolyVerts.Num()]);

					//maybe offset z abit so it is can be seen when debug
					Node->SetNodeLocation(Middle/* + FVector(0.f, 0.f, 10.f)*/);

					NodeGraph.NodeMap.Add(IntMiddle, *Node);
				}
				SpawnNodes.Add(IntMiddle);
			}
			//set neighbour for node from this tile
			for (const FIntVector NodeA : SpawnNodes)
			{
				for(const FIntVector NodeB : SpawnNodes)
				if (NodeA != NodeB)
				{
					NodeGraph.NodeMap[NodeA].AddNeighbor(NodeB);
				}
			}
		}
	}

	if (SkipCount == NavMeshTileCount)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("Probably no navigation mesh has been generated. Please regenerate the navigation mesh."));
	}
}

void AInfGraph::ConnectingNodeIslands()
{
}

void AInfGraph::ClearGraph()
{
	NodeGraph.Reset();
}

const FInfMap* AInfGraph::GetNodeGraphData() const
{
	return &NodeGraph;
}

const FInfNode* AInfGraph::GetNode(const FIntVector& Key) const
{
	return NodeGraph.NodeMap.Contains(Key) ? &NodeGraph.NodeMap[Key] : nullptr;
}

const FVector AInfGraph::GetNodeLocation(const FIntVector& Key) const
{
	return NodeGraph.NodeMap.Contains(Key) ? NodeGraph.NodeMap[Key].GetNodeLocation() : FVector(0);
}

void AInfGraph::DrawDebugNodeGraph(bool bDrawConnectingNeighbor) const
{
	FVector HeightOffset = FVector(0.f, 0.f, 40.f);
	for (auto Pair : NodeGraph.NodeMap)
	{
		const FInfNode* Node = &Pair.Value;

		if(!InDebugRange(Node->GetNodeLocation())) continue;
			
		DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + HeightOffset, 20.f, FNavMeshRenderingHelpers::GetClusterColor(Node->GetRegionTileID()), true, -1.f);

		if (bDrawConnectingNeighbor)
		{
			for (const FIntVector Neighbor : Node->GetNeighbor())
			{
				if (!NodeGraph.NodeMap.Contains(Neighbor))
					continue;
				FVector LineEnd = Node->GetNodeLocation() + (GetNodeLocation(Neighbor) - Node->GetNodeLocation()) *
					0.4f;
				DrawDebugDirectionalArrow(GetWorld(), Node->GetNodeLocation()+ HeightOffset, LineEnd+ HeightOffset, 50.f, FColor::Green, true, -1.f,
				                          0, 1.5f);
			}
		}
	}
}

bool AInfGraph::InDebugRange(const FVector& Location) const
{
	if (!GetWorld()) return true;
	if (GetWorld()->ViewLocationsRenderedLastFrame.Num() == 0) return true;
	return FVector::Dist(GetWorld()->ViewLocationsRenderedLastFrame[0], Location) < 10000;
}

FInfNode* AInfGraph::FindNearestNode(const FVector& FeetLocation) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AInfNodeGraph_FindNearestNode);

	check(NodeGraph.NodeMap.Num() > 0)
	
	/*int TargetTileIdx = FindNavmeshTilesContainsLocation(FeetLocation);
	if (TargetTileIdx == INDEX_NONE)
		return nullptr;
	
	const FIntVector NearestNodeKey = FindNearestNodeKey(TargetTileIdx, FeetLocation);
	if (NearestNodeKey == FIntVector::NoneValue)
		return nullptr;

	FInfNode Node = NodeGraph.NodeMap[NearestNodeKey];
	return &Node;*/
	
	NavNodeRef NavNode = NavMeshCache->FindNearestPoly(FeetLocation,
		FVector(0,0, NavMeshCache->AgentHeight),
		NavMeshCache->GetDefaultQueryFilter());
	
	TArray<FVector> PolyVerts;
	NavMeshCache->GetPolyVerts(NavNode, PolyVerts);
	
	if(PolyVerts.Num() < 1) return nullptr;

	float MinDistSq = FLT_MAX;
	FIntVector NearestNodeKey(0);
	
	for (int VertIdx = 0; VertIdx < PolyVerts.Num(); VertIdx++)
	{
		FVector Middle = (PolyVerts[VertIdx] + PolyVerts[(VertIdx + 1) % PolyVerts.Num()]) * 0.5f;

		float DistSq = FVector::DistSquared(FeetLocation, Middle);
		if (MinDistSq > DistSq)
		{
			MinDistSq = DistSq;

			NearestNodeKey = FIntVector(Middle);
		}
	}
	const FInfNode* Node = &NodeGraph.NodeMap[NearestNodeKey];
	return const_cast<FInfNode*>(Node);
}

int AInfGraph::FindNavmeshTilesContainsLocation(const FVector& FeetLocation) const
{
	const int ActiveNavMeshTileCount = NavMeshCache->GetNavMeshTilesCount();
	for (int TileSetIdx = 0; TileSetIdx < ActiveNavMeshTileCount; TileSetIdx++)
	{
		FBox TileBounds = NavMeshCache->GetNavMeshTileBounds(TileSetIdx);
		if (!TileBounds.IsValid)
			continue;
		
		FVector QueryExtent(2.f, 2.f, NavMeshCache->AgentHeight * 100.f);
		FBox QueryBounds(FeetLocation - QueryExtent, FeetLocation + QueryExtent);

		//TODO: Fix this bound bounding thing
		if (!TileBounds.Intersect(QueryBounds))
			continue;

		return TileSetIdx;
	}

	// ‚±‚±‚É“ž’B‚µ‚½‚Æ‚«ƒG[ƒWƒFƒ“ƒg‚ÍƒiƒrƒƒbƒVƒ…ŠO‚É‚¢‚é.
	return INDEX_NONE;
}

FIntVector AInfGraph::FindNearestNodeKey(int TargetTileIdx, const FVector& FeetLocation) const
{
	FIntVector NearestNodeKey(0);
	
	TArray<FNavPoly> Polys;
	if (!NavMeshCache->GetPolysInTile(TargetTileIdx, Polys))
		return NearestNodeKey;
	
	float MinDistSq = FLT_MAX;
	for (const FNavPoly& Poly : Polys)
	{
		TArray<FVector> PolyVerts;
		if (!NavMeshCache->GetPolyVerts(Poly.Ref, PolyVerts))
			continue;

		for (int VertIdx = 0; VertIdx < PolyVerts.Num(); VertIdx++)
		{
			FVector Middle = (PolyVerts[VertIdx] + PolyVerts[(VertIdx + 1) % PolyVerts.Num()]) * 0.5f;

			float DistSq = FVector::DistSquared(FeetLocation, Middle);
			if (MinDistSq > DistSq)
			{
				MinDistSq = DistSq;

				NearestNodeKey = FIntVector(Middle);
			}
		}
	}

	return NearestNodeKey;
}
