// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Navigation/InfNavMesh.h"
#include "NavMesh/RecastHelpers.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/AITask_InfMapMoveTo.h"

DECLARE_CYCLE_STAT(TEXT("Custom Pathfinding"), STAT_Navigation_CustomPathfinding, STATGROUP_Navigation)

FInfQueryFilter::FInfQueryFilter(bool inIsVirtual)
: FRecastQueryFilter(inIsVirtual)
	, InfluenceMap(nullptr)
	, FIXED_ADDITIONAL_COST(1000.f)
	, CostMultiplier(1.f)
	, World(nullptr)
	, bDrawDebugFindPath(false)
{
}

void FInfQueryFilter::SetDataUsedForDebugging(UWorld* NewWorld, bool bDrawDebugEnabled)
{
	World = NewWorld;
	bDrawDebugFindPath = bDrawDebugEnabled;
	UE_LOG(LogTemp, Warning, TEXT("FInfQueryFilter SetAdditionalData"));
}

void FInfQueryFilter::SetAdditionalNavigationData(const TMap<FIntVector, float>& InInfluenceMapData,
	float InCostMultiplier)
{
	InfluenceMap = &InInfluenceMapData;
	CostMultiplier = InCostMultiplier;
	// �����̓f�o�b�O�p�r�̗]�v�ȏ���.
	if (bDrawDebugFindPath)
	{
		FlushPersistentDebugLines(World);
		FlushDebugStrings(World);
	}
}

void FInfQueryFilter::Reset()
{
	UE_LOG(LogTemp, Warning, TEXT("FInfQueryFilter Reset"));
	FInfQueryFilter* Filter = static_cast<FInfQueryFilter*>(this);

	Filter = new(Filter)FInfQueryFilter(true);
	SetExcludedArea(RECAST_NULL_AREA);
}

bool FInfQueryFilter::IsEqual(const INavigationQueryFilterInterface* Other) const
{
	return FMemory::Memcmp(this, Other, sizeof(FInfQueryFilter)) == 0;
}

float FInfQueryFilter::getVirtualCost(const float* pa, const float* pb, const dtPolyRef prevRef,
	const dtMeshTile* prevTile, const dtPoly* prevPoly, const dtPolyRef curRef, const dtMeshTile* curTile,
	const dtPoly* curPoly, const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const
{
#if WITH_FIXED_AREA_ENTERING_COST

	float areaChangeCost = 0.f;
	if (nextPoly != 0 && nextPoly->getArea() != curPoly->getArea())
		areaChangeCost = data.m_areaFixedCost[nextPoly->getArea()];

	float AdditionalCost = 0.f;
	if (InfluenceMap && InfluenceMap->Num() != 0)
	{
		FIntVector NodeALoc = FIntVector(Recast2UnrealPoint(pa));
		FIntVector NodeBLoc = FIntVector(Recast2UnrealPoint(pb));
		
		if (InfluenceMap->Contains(NodeALoc)) { AdditionalCost = (*InfluenceMap)[NodeALoc] * CostMultiplier * FIXED_ADDITIONAL_COST; }
		if (InfluenceMap->Contains(NodeBLoc)) { AdditionalCost = (*InfluenceMap)[NodeBLoc] * CostMultiplier * FIXED_ADDITIONAL_COST; }
	}

	float TraversalCost = dtVdist(pa, pb) * data.m_areaCost[curPoly->getArea()] + areaChangeCost + AdditionalCost;

	if (bDrawDebugFindPath)
	{
		FVector Begin = Recast2UnrealPoint(pa);
		FVector End = Recast2UnrealPoint(pb);
		DrawDebugSphere(World, Begin, 15.f, 8, FColor::Magenta, true, 1.f);
		DrawDebugSphere(World, End, 15.f, 8, FColor::Magenta, true, 1.f);
		DrawDebugDirectionalArrow(World, Begin, End, 100.f, FColor::Cyan, true, -1.f, 0, 1.f);

		FVector TextLocation = (Begin + End) * 0.5f;
		DrawDebugString(World, TextLocation + FVector(-30.f, 0.f, 0.f), FString::SanitizeFloat(TraversalCost));
	}

	return TraversalCost;
#else
	return dtVdist(pa, pb) * data.m_areaCost[curPoly->getArea()];
#endif // #if WITH_FIXED_AREA_ENTERING_COST
}

void AInfNavMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfNavMesh, bDrawDebugFindPath))
	{
		RecreateDefaultFilter();
	}
}

void AInfNavMesh::RecreateDefaultFilter()
{
	UE_LOG(LogTemp, Warning, TEXT("AInfluenceRecastNavmesh RecreateDefaultFilter"));

	DefaultQueryFilter->SetFilterType<FInfQueryFilter>();
	DefaultQueryFilter->SetMaxSearchNodes(DefaultMaxSearchNodes);
	
	DetourFilter = static_cast<FInfQueryFilter*>(DefaultQueryFilter->GetImplementation());
	DetourFilter = new(DetourFilter)FInfQueryFilter(true);
	DetourFilter->SetDataUsedForDebugging(GetWorld(), bDrawDebugFindPath);
	
	DetourFilter->setHeuristicScale(HeuristicScale);
	DetourFilter->setIncludeFlags(DetourFilter->getIncludeFlags() & (~AInfNavMesh::GetNavLinkFlag()));

	for (int32 Idx = 0; Idx < SupportedAreas.Num(); Idx++)
	{
		const FSupportedAreaData& AreaData = SupportedAreas[Idx];

		UNavArea* DefArea = nullptr;
		if (AreaData.AreaClass)
		{
			DefArea = ((UClass*)AreaData.AreaClass)->GetDefaultObject<UNavArea>();
			if (DefArea)
			{
				DetourFilter->SetAreaCost(AreaData.AreaID, DefArea->DefaultCost);
				DetourFilter->SetFixedAreaEnteringCost(AreaData.AreaID, DefArea->GetFixedAreaEnteringCost());
			}
		}
	}
}

void AInfNavMesh::SetInfluenceMapData(const FAIInfMapMoveRequest& InfluenceMapData)
{
	if ((*InfluenceMapData.GetTargetInfluenceMapData()).Num() > 0)
	{
		DetourFilter->SetAdditionalNavigationData((*InfluenceMapData.GetTargetInfluenceMapData()), InfluenceMapData.GetCostMultiplier());
	}
}

void AInfNavMesh::OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles)
{
	Super::OnNavMeshTilesUpdated(ChangedTiles);

	//send this array to the influence graph
	OnNeedToUpdateGraph.Broadcast(ChangedTiles);
}
