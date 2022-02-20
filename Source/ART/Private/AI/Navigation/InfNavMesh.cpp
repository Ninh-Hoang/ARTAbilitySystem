// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Navigation/InfNavMesh.h"
#include "NavMesh/RecastHelpers.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/AITask_InfMapMoveTo.h"
#include "AI/Navigation/InfCollectionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "VisualLogger/VisualLogger.h"

DECLARE_CYCLE_STAT(TEXT("Custom Pathfinding"), STAT_Navigation_CustomPathfinding, STATGROUP_Navigation)

FInfQueryFilter::FInfQueryFilter(bool inIsVirtual)
: FRecastQueryFilter(inIsVirtual)
	, InfluenceMap()
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

void FInfQueryFilter::SetAdditionalNavigationData(const FInfQueryData& QueryData,
	float InCostMultiplier)
{
	const FMapOperationResult ResultData = UInfMapFunctionLibrary::GetInfluenceMapFromQuery(QueryData);
	InfluenceMap = ResultData.ResultMap;	
	
	CostMultiplier = InCostMultiplier;
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

dtReal FInfQueryFilter::getVirtualCost(const dtReal* pa, const dtReal* pb,
		const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
		const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly,
		const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const
{
#if WITH_FIXED_AREA_ENTERING_COST

	float areaChangeCost = 0.f;
	if (nextPoly != 0 && nextPoly->getArea() != curPoly->getArea())
		areaChangeCost = data.m_areaFixedCost[nextPoly->getArea()];

	float AdditionalCost = 0.f;
	if (InfluenceMap.Num() != 0)
	{
		FIntVector NodeALoc = FIntVector(Recast2UnrealPoint(pa));
		FIntVector NodeBLoc = FIntVector(Recast2UnrealPoint(pb));
		
		if (InfluenceMap.Contains(NodeALoc)) { AdditionalCost = InfluenceMap[NodeALoc] * CostMultiplier * FIXED_ADDITIONAL_COST; }
		if (InfluenceMap.Contains(NodeBLoc)) { AdditionalCost = InfluenceMap[NodeBLoc] * CostMultiplier * FIXED_ADDITIONAL_COST; }
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

void AInfNavMesh::SetInfluenceQueryData(const FAIInfMapMoveRequest& InfluenceMapData)
{
	if ((InfluenceMapData.GetInfluenceQueryData()->OperationConstructData).Num() > 0)
	{
		DetourFilter->SetAdditionalNavigationData((*InfluenceMapData.GetInfluenceQueryData()), InfluenceMapData.GetCostMultiplier());
	}
}

void AInfNavMesh::OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles)
{
	Super::OnNavMeshTilesUpdated(ChangedTiles);

	//send this array to the influence graph
	OnNeedToUpdateGraph.Broadcast(ChangedTiles);
}

void AInfNavMesh::BeginPlay()
{
	Super::BeginPlay();

	//try to get InfCollectionActor from world
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithInterface(this, UInfCollectionInterface::StaticClass(), OutActors);

	if (OutActors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT(" UInfluencePropagator::BeginPlay(): Influence Map Collection does not exist"));
		return;
	}

	IInfCollectionInterface* MapCollectionInterface = Cast<IInfCollectionInterface>(OutActors[0]);
	InfluenceMapCollectionRef = MapCollectionInterface ;
	InfluenceMapCollectionInterfaceRef = MapCollectionInterface->_getUObject();
}

void AInfNavMesh::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	NavigationTickTime -= DeltaSeconds;
	if(NavigationTickTime < 0 || !InfluenceMapCollectionRef)
	{
		NavigationTickTime = 1.f;

		const int32 PathsCount = ActivePaths.Num();
		FScopeLock PathLock(&ActivePathsLock);

		FNavPathWeakPtr* WeakPathPtr = (ActivePaths.GetData() + PathsCount - 1);
		for (int32 PathIndex = PathsCount - 1; PathIndex >= 0; --PathIndex, --WeakPathPtr)
		{
			FNavPathSharedPtr SharedPath = WeakPathPtr->Pin();
			if (WeakPathPtr->IsValid() == false)
			{
				ActivePaths.RemoveAtSwap(PathIndex, 1, /*bAllowShrinking=*/false);
			}
			else
			{
				// iterate through all tile refs in FreshTilesCopy and 
				FNavMeshPath* Path = (FNavMeshPath*)(SharedPath.Get());
				if (Path->IsReady() == false ||
					Path->GetIgnoreInvalidation() == true)
				{
					// path not filled yet or doesn't care about invalidation
					continue;
				}

				//make sure it is influence movement query filter
				//TODO: this is some evil taboo stuffs, can this be improved?
				FNavigationQueryFilter* Query = const_cast<FNavigationQueryFilter*>(Path->GetQueryData().QueryFilter.Get());
				const FInfQueryFilter* InfluenceFilter = static_cast<FInfQueryFilter*>(Query->GetImplementation());

				if(!InfluenceFilter) continue;;

				const TArray<uint32>& AffectedTiles = InfluenceMapCollectionRef->GetAffectedTile();
				const int32 PathLenght = Path->PathCorridor.Num();
				const NavNodeRef* PathPoly = Path->PathCorridor.GetData();

				for (int32 NodeIndex = 0; NodeIndex < PathLenght; ++NodeIndex, ++PathPoly)
				{
					uint32 PolyIndex = NodeIndex;
					uint32 NodeTileIdx; 
					GetPolyTileIndex(*PathPoly, PolyIndex, NodeTileIdx);
					if (AffectedTiles.Contains(NodeTileIdx))
					{
						SharedPath->Invalidate();
						ActivePaths.RemoveAtSwap(PathIndex, 1, false);
						break;
					}
				}
			}
		}
	}
}
