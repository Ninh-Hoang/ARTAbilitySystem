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
	if(NavMeshCache)
	{
		NavMeshCache->OnNeedToUpdateGraph.AddDynamic(this, &AInfGraph::OnNeedUpdateGraph);
	}
}

void AInfGraph::OnNeedUpdateGraph(const TArray<uint32>& ChangedTiles)
{
#if WITH_EDITOR
	const auto StartTime = std::chrono::high_resolution_clock::now();
#endif
	//remove node from map
	TArray<FIntVector> NodeInChangedTile;
	
	NodeInChangedTile.Reserve(NodeGraph.NodeMap.Num());

	//get all node for removal
	for (auto& Pair : NodeGraph.NodeMap)
	{
		const FInfNode* Node = &Pair.Value;
		bool NodeIsEncapsulated = false;
		
		for(int32 TileID :Node->GetRegionTileID())
		{
			if(ChangedTiles.Contains(TileID))
			{
				NodeInChangedTile.Add(Pair.Key);
				break;
			}
		}
	}

	//get key to actually remove and get edge node
	TArray<FIntVector> KeyToRemove;
	TArray<FIntVector> KeyAtEdge;
	KeyToRemove.Reserve(NodeInChangedTile.Num());
	KeyAtEdge.Reserve(NodeInChangedTile.Num());
	
	for(FIntVector& NodeIndex : NodeInChangedTile)
	{
		FInfNode* Node = &NodeGraph.NodeMap[NodeIndex];
		bool NodeIsAtEdge = false;
		for(int32 TileID : Node->GetRegionTileID())
		{
			if(!ChangedTiles.Contains(TileID))
			{
				NodeIsAtEdge = true;

				//also remove node from this tile and set it this the tile that not belong to changed tile
				Node->ResetRegionTileID();
				Node->AddRegionTileID(TileID);
				break;
			}
		}
		if(!NodeIsAtEdge)
		{
			KeyToRemove.Add(NodeIndex);
		}
		else
		{
			KeyAtEdge.Add(NodeIndex);
		}
	}

	//we have to keep node that have access to neighbors that do not belong to changed tile
	//remove all other node, access their neighbour and remove
	TArray<FIntVector> NeighborToRemove;
	for (int32 i = 0; i < KeyAtEdge.Num(); i++)
	{
		FInfNode& Node = NodeGraph.NodeMap[KeyAtEdge[i]];
		DrawDebugPoint(GetWorld(), Node.GetNodeLocation(), 50.f, FColor::Red, false, 5.f);
		
		//access all neighbor, get their neighbour list and remove this node graph location from it
		const TArray<FIntVector>& Neighbors = Node.GetNeighbor();
		
		for(int32 a = 0; a < Neighbors.Num(); a++)
		{
			
			if(NodeInChangedTile.Contains(Neighbors[a]))
			{
				NeighborToRemove.Add(Neighbors[a]);
			}
		}

		for(auto& Neighbor : NeighborToRemove)
		{
			Node.RemoveNeighbor(Neighbor);
		}

		//we do this so we don't have to make a new array all the time
		NeighborToRemove.Reset();
	}
	
	//finally remove node if not edge node
	for (int32 i = 0; i < KeyToRemove.Num(); i++)
	{
		NodeGraph.NodeMap.Remove(KeyToRemove[i]);
	}
	
	//add new node to map
	int NewID = 0, SkipCount = 0;

	TArray<FIntVector> SpawnNodes;
	for(int32 i = 0; i < ChangedTiles.Num(); i++)
	{
		const uint32 CurrentTileIndex = ChangedTiles[i];
		
		TArray<FNavPoly> Polys;
		if (!NavMeshCache->GetPolysInTile(CurrentTileIndex, Polys))
		{
			continue;
		}

		for (const FNavPoly& Poly : Polys)
		{
			TArray<FVector> PolyVerts;
			
			//skip if this poly has no vertex
			if (!NavMeshCache->GetPolyVerts(Poly.Ref, PolyVerts))
			{
				SkipCount++;
				continue;
			}
			
			// iterate verts
			for (int VertIdx = 0; VertIdx < PolyVerts.Num(); VertIdx++)
			{
				FVector Middle = (PolyVerts[VertIdx] + PolyVerts[(VertIdx + 1) % PolyVerts.Num()]) * 0.5f;
				FIntVector IntMiddle = FIntVector(Middle);

				FInfNode* Node;
				if(!NodeGraph.NodeMap.Contains(IntMiddle))
				{
					FInfNode TempNode = FInfNode();
					Node = &TempNode;
					Node->AddRegionTileID(CurrentTileIndex);
					Node->SetGraphLocation(IntMiddle);
					Node->SetNodeLocation(Middle);

					NodeGraph.NodeMap.Add(IntMiddle, *Node);
				}
				else
				{
					Node = &NodeGraph.NodeMap[IntMiddle];
					Node->AddRegionTileID(CurrentTileIndex);
				}
				SpawnNodes.Add(IntMiddle);
			}
			
			//set neighbour for node from this tile
			for (const FIntVector& NodeA : SpawnNodes)
			{
				for(const FIntVector& NodeB : SpawnNodes)
					if (NodeA != NodeB)
					{
						NodeGraph.NodeMap[NodeA].AddNeighbor(NodeB);
					}
			}
		}
		SpawnNodes.Reset();
	}

	if (SkipCount == ChangedTiles.Num())
	{
		UE_LOG(LogTemp, Error,
			   TEXT("Probably no navigation mesh was re-generated."));
	}
	
#if WITH_EDITOR
	const float Duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count();
	int32 NodeCount = NodeGraph.NodeMap.Num();
	int32 NeighborLinkCount = 0;
	for (auto& Pair : NodeGraph.NodeMap)
	{
		const FInfNode* Node = &Pair.Value;
		NeighborLinkCount += Node->GetNeighbor().Num();
	}
	
	UE_LOG(LogTemp, Display, TEXT("Re-generate time: %f miliseconds"), Duration);
	UE_LOG(LogTemp, Display, TEXT("New total Node Count : %i"), NodeCount);
	UE_LOG(LogTemp, Display, TEXT("Total Node Neighbor Link Count : %i"), NeighborLinkCount);
#endif
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
		FlushDebugStrings(GetWorld());
		DrawDebugNodeGraph();
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
	const float Duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count();
	int32 NumBytes = NodeGraph.GetSize();
	int32 NodeCount = NodeGraph.NodeMap.Num();
	int32 NeighborLinkCount = 0;
	for (auto& Pair : NodeGraph.NodeMap)
	{
		const FInfNode* Node = &Pair.Value;
		NeighborLinkCount += Node->GetNeighbor().Num();
	}
	
	UE_LOG(LogTemp, Display, TEXT("Generation Time : %f miliseconds"), Duration);
	UE_LOG(LogTemp, Display, TEXT("Total Graph btyes : %i"), NumBytes);
	UE_LOG(LogTemp, Display, TEXT("Total Node Count : %i"), NodeCount);
	UE_LOG(LogTemp, Display, TEXT("Total Node Neighbor Link Count : %i"), NeighborLinkCount);
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

					Node->AddRegionTileID(TileSetIdx);
					Node->SetGraphLocation(IntMiddle);

					//maybe offset z abit so it is can be seen when debug
					Node->SetNodeLocation(Middle/* + FVector(0.f, 0.f, 10.f)*/);

					NodeGraph.NodeMap.Add(IntMiddle, *Node);
				}
				else
				{
					Node = &NodeGraph.NodeMap[IntMiddle];
					Node->AddRegionTileID(TileSetIdx);
				}
				SpawnNodes.Add(IntMiddle);
			}
			//set neighbour for node from this tile
			for (const FIntVector& NodeA : SpawnNodes)
			{
				for(const FIntVector& NodeB : SpawnNodes)
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

void AInfGraph::DrawDebugNodeGraph() const
{
	const FVector HeightOffset = FVector(0.f, 0.f, 40.f);
	
	if(DrawDebugType == EDebugMenu::SINGLE_NODE)
	{
		if(NodeGraph.NodeMap.Find(NodeToDraw))
		{
			const FInfNode* Node = &NodeGraph.NodeMap[NodeToDraw];
			DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + HeightOffset, 20.f, FColor::Green, true, -1.f);
			DrawDebugString(GetWorld(), Node->GetNodeLocation(), *Node->GetGraphLocation().ToString(), nullptr, FColor::White, 99.f);

			for(auto& Neighbor : Node->GetNeighbor())
			{
				const FInfNode* NeighborNode = &NodeGraph.NodeMap[Neighbor];
				DrawDebugPoint(GetWorld(), NeighborNode->GetNodeLocation() + HeightOffset, 20.f, FColor::Emerald, true, -1.f);
				DrawDebugString(GetWorld(), NeighborNode->GetNodeLocation(), *NeighborNode->GetGraphLocation().ToString(), nullptr, FColor::White, 99.f);
			}
		}
	}
	else
	{
		for (auto& Pair : NodeGraph.NodeMap)
		{
			const FInfNode* Node = &Pair.Value;

			if(!InDebugRange(Node->GetNodeLocation())) continue;
			
			DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + HeightOffset, 20.f, FColor::Green, true, -1.f);
			DrawDebugString(GetWorld(), Node->GetNodeLocation(), *Node->GetGraphLocation().ToString(), nullptr, FColor::White, 99.f);

			if (DrawDebugType == EDebugMenu::NODES_NEIGHBOR)
			{
				for (const FIntVector Neighbor : Node->GetNeighbor())
				{
					/*if (!NodeGraph.NodeMap.Contains(Neighbor))
						continue;*/
					FVector LineEnd = Node->GetNodeLocation() + (GetNodeLocation(Neighbor) - Node->GetNodeLocation()) *
						0.4f;
					DrawDebugDirectionalArrow(GetWorld(), Node->GetNodeLocation()+ HeightOffset, LineEnd+ HeightOffset, 50.f, FColor::Blue, true, -1.f,
											  0, 1.5f);
				}
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

	//TODO: Navigation data update, the index might be wrong
	if(!NodeGraph.NodeMap.Contains(NearestNodeKey)) return nullptr;
	
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
