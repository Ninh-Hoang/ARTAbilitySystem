// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Navigation/InfPropagator.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/InfCollectionInterface.h"
#include "AI/Navigation/InfMapInterface.h"
#include "AI/Navigation/InfGraphInterface.h"
#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "chrono"

// Sets default values for this component's properties
UInfPropagator::UInfPropagator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bActivate = false;
	PropagateRange = 500.f;
	AttenuationRatio = 1.f;
}

// Called when the game starts
void UInfPropagator::BeginPlay()
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
	
	PreviousPropagatorTransform = FTransform(FVector(99999.f, 99999.f, 99999.f));

	//calculate head and feet offset based off actor bound
	FVector BoundsOrigin, BoundsExtents;
	GetOwner()->GetActorBounds(true, BoundsOrigin, BoundsExtents);
	OwnerHeadOffset = FVector(0.f, 0.f, BoundsExtents.Z);
	OwnerFeetOffset = FVector(0.f, 0.f, -BoundsExtents.Z);

	//initialize propagator
	Initialize(Cast<IInfCollectionInterface>(OutActors[0]));
}

/*void UInfPropagator::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	IGenericTeamAgentInterface::SetGenericTeamId(TeamID);
}

FGenericTeamId UInfPropagator::GetGenericTeamId() const
{
	if (const IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(GetOwner()))
	{
		return TeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId();
}

ETeamAttitude::Type UInfPropagator::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(GetOwner()))
	{
		return TeamInterface->GetTeamAttitudeTowards(Other);
	}
	return ETeamAttitude::Neutral;
}*/

FGenericTeamId UInfPropagator::GetTeam() const
{
	if (const IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(GetOwner()))
	{
		return TeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId();
}

void UInfPropagator::Initialize(IInfCollectionInterface* InfluenceMapCollection)
{
	//get a reference to the InfCollection
	InfluenceMapCollectionRef = InfluenceMapCollection;
	InfluenceMapCollectionInterfaceRef = InfluenceMapCollection->_getUObject();

	//try to get the target map from the collection
	auto* TargetMap = InfluenceMapCollectionRef->GetMapSafe(TargetMapTag);
	if (TargetMap == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::Initialize: %s does not exist"), *TargetMapTag.ToString());
		return;
	}

	//register this propagator with the map
	TargetMap->AddPropagator(this);

	//get base graph data from the collection 
	const auto& GraphData = InfluenceMapCollectionRef->GetNodeGraph()->GetNodeGraphData();
	if (GraphData->NodeMap.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::Initialize: Node graph is empty"), *TargetMapTag.ToString());
		return;
	}

	//reserve space for the propagation map base on the number of node from base graph
	const int INFLUENCE_MAP_SIZE = GraphData->NodeMap.Num();
	RecentPropagationMap.Reserve(INFLUENCE_MAP_SIZE);
	MergedPropagationMap.Reserve(INFLUENCE_MAP_SIZE);
	PropagationMapHistory.Reserve((1 / AttenuationRatio) + 1);
}

void UInfPropagator::UpdatePropagationMap()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UInfPropagator_UpdatePropagationMap);
	
/*#if WITH_EDITOR
	const auto StartTime = std::chrono::high_resolution_clock::now();
#endif*/
	
	//if not activated skip
	if (!bActivate)
	{
		MergedPropagationMap.Reset();
		return;
	}

	//if history map is valid, do attenuation
	if (PropagationMapHistory.Num() > 0)
		AttenuationPropagationMaps();

	//try get current node location
	FVector FeetLocation = GetOwner()->GetActorLocation() + OwnerFeetOffset;
	const FInfNode* FeetNode = InfluenceMapCollectionRef->GetNodeGraph()->FindNearestNode(FeetLocation);

	if (FeetNode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdatePropagationMap() : Failed to find nearest node. Need rebuild NodeGraph & Influence Map"));
		return;
	}

	//add recent propagation map to history if required, else, create a new map and add it
	if (PropagationMapHistory.Num() > 0 && (!ShouldUpdatePropagationMap()))
	{
		PropagationMapHistory.Add(FPropagationMap(RecentPropagationMap, 1.f));
	}
	else
	{
		RecentPropagationMap.Reset();
		
		RecentPropagationMap = CreateNewPropagationMap(FeetNode);

		//add to history
		PropagationMapHistory.Add(FPropagationMap(RecentPropagationMap, 1.f));

		//PreviousPropagateCenter = FeetLocation;
		PreviousPropagatorTransform = FTransform(GetOwner()->GetActorRotation(), FeetLocation, GetOwner()->GetActorScale());
	}

	//finish updating all map, merging map
	MergePropagationMaps();
		
/*#if WITH_EDITOR
    const float Duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - StartTime).count() / 1000.0f;
	UE_LOG(LogTemp, Display, TEXT("Map update Time : %f microseconds"), Duration);
#endif*/
}

void UInfPropagator::SetPropagationActivation(bool InActivate)
{
	bActivate = InActivate;
}

TMap<FIntVector, float> UInfPropagator::CreateNewPropagationMap(const FInfNode* CenterNode) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UInfPropagator_CreateNewPropagationMap);

	const auto* NodeGraphRef = InfluenceMapCollectionRef->GetNodeGraph();
	FHitResult TraceResult;
	
	auto PropagationCalculator = [&](uint32 StepDistance, uint32 MaxStepDistance, const FVector& NodeLocation)
	{
		//TODO: potentially has to do a trace to see obstruction
		/*GetWorld()->LineTraceSingleByChannel(TraceResult, 
											 GetOwner()->GetActorLocation() + OwnerHeadOffset, 
											 NodeLocation, 
											 ECollisionChannel::ECC_Visibility, // Pawn‚âCharacterMesh‚É‚Íƒqƒbƒg‚µ‚È‚¢ƒ`ƒƒƒ“ƒlƒ‹‚ðŽw’è.
											 FCollisionQueryParams{ "Propagate LOS", false, GetOwner() });
		
		*/
		float Ratio = 0.f;
		
		/*if (!TraceResult.bBlockingHit || (TraceResult.GetActor() && TraceResult.GetActor()->ActorHasTag("Node")))
			Ratio = FVector::Dist(CenterNode->GetActorLocation(), NodeLocation) / PropagateRange;
		else*/

		//Ratio = FVector::Dist(CenterNode->GetNodeLocation(), NodeLocation) / PropagateRange;
		//TODO: we are using step to simplify the value calculation,
		//TODO: thanks to the uneven distribution of the navmesh, distance between step is not always the same
		Ratio = (float)StepDistance / MaxStepDistance;

		//use curve if have
		if (PropagationCurve) return PropagationCurve->GetFloatValue(Ratio);

		return FMath::Max(0.0f, 1.f - 1.f * Ratio);

	};
	//TODO: Don't think we need this
	auto ExcludeFromPropagationCalc = [&](const AActor* TargetNodeActor)
	{
		return true;
	};


	return CreateNewMap(CenterNode, PropagateRange, PropagationCalculator, ExcludeFromPropagationCalc);
}

bool UInfPropagator::ShouldUpdatePropagationMap() const
{
	return FarAwayFrom(PreviousPropagatorTransform.GetLocation());
}

bool UInfPropagator::FarAwayFrom(const FVector& PreviousLocation) const
{
	FVector FeetLocation = GetOwner()->GetActorLocation() + OwnerFeetOffset;

	// far enough or not to update, square to save computational power
	return (FVector::DistSquared(FeetLocation, PreviousLocation) > FarawayDistanceToleranceSq);
}

void UInfPropagator::AttenuationPropagationMaps()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UInfPropagator_AttenuationPropagationMaps);

	const float ATTENUATION_RATIO = AttenuationRatio;
	int RemoveTargetIndex = -1;
	TArray<int> RemoveList;
	RemoveList.Reserve(PropagationMapHistory.Num());

	//remove history map with MaxValue near 0
	//lower value of node in the map
	for (FPropagationMap& StampData : PropagationMapHistory)
	{
		++RemoveTargetIndex;
		if (FMath::IsNearlyZero(StampData.MaxValue, 0.001f))
		{
			RemoveList.Add(RemoveTargetIndex);
			continue;
		}
	
		StampData.MaxValue -= ATTENUATION_RATIO;
		for (auto& Pair : StampData.MapData)
			Pair.Value = FMath::Max(Pair.Value * StampData.MaxValue, 0.f);
	}

	for (int Index : RemoveList)
		PropagationMapHistory.RemoveAt(Index, 1, false);
}

void UInfPropagator::MergePropagationMaps()
{
	if (PropagationMapHistory.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("PropagationMapHistory is empty !!"));
		return;
	}

	MergedPropagationMap.Reset();

	//add all history map together
	for (const FPropagationMap& StampData : PropagationMapHistory)
	{
		for (const auto& Pair : StampData.MapData)
		{
			if (MergedPropagationMap.Contains(Pair.Key))
				MergedPropagationMap[Pair.Key] += Pair.Value;
			else
				MergedPropagationMap.Add(Pair.Key, Pair.Value);
		}
	}
}

TMap<FIntVector, float> UInfPropagator::CreateNewMap(const FInfNode* CenterNode, float MaxRange,
	PropagationValueCalculator PropagationValueFunc, ExcludeFromPropagationValueCalc ExcludeFunc) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UInfPropagator_CreateNewMap);
	
	// get map and size
	const auto& InfluenceGraph = InfluenceMapCollectionRef->GetNodeGraph()->GetNodeGraphData();
	const TMap<FIntVector, FInfNode>* InfluenceMap = &InfluenceGraph->NodeMap;
	const int INFLUENCEMAP_SIZE = InfluenceMap->Num();
	
	//pick only node that are needed for value
	TArray<TPair<FIntVector, uint32>> WorkingBuffer, NeedPropagationCuration, CacheNeighbors;
	TArray<FIntVector> MarkedForPropagation;
	
	WorkingBuffer.Reserve(INFLUENCEMAP_SIZE);
	NeedPropagationCuration.Reserve(INFLUENCEMAP_SIZE);
	MarkedForPropagation.Reserve(INFLUENCEMAP_SIZE);

	//write centre node in to the array
	WorkingBuffer.Emplace(CenterNode->GetGraphLocation(), 0);
	NeedPropagationCuration.Emplace(CenterNode->GetGraphLocation(), 0);

	//first step, set as 0
	uint32 MaxStep = 0;
	auto CurrentNode = TPair<FIntVector, uint32>();

	//create map to cache visited node and result
	TMap<FIntVector, bool> Visited;
	Visited.Reserve(INFLUENCEMAP_SIZE);

	TMap<FIntVector, float> Result;
	Result.Reserve(INFLUENCEMAP_SIZE);

	//cache from value that we will be using many time
	FVector CentreLocation = CenterNode->GetNodeLocation();
	float MaxRangeSquared = FMath::Square(MaxRange);
	
	// while there are still node to works on
	while (WorkingBuffer.Num() != 0)
	{
		// get a node and also remove it from working list
		CurrentNode = WorkingBuffer.Pop(false);
		
		//TODO: what the fuck is this
		// if this node is already visited, skip to next one
		if (Visited.Find(CurrentNode.Key) /*|| !ExcludeFunc(Cast<AActor>(InfluenceMap[]))*/)
			continue;

		//write into visited list
		Visited.Emplace(CurrentNode.Key, true);

		//for each neighbour of current node, check if they are in range
		const TArray<FIntVector> Neighbors = InfluenceMap->Find(CurrentNode.Key)->GetNeighbor();
		
		for (const FIntVector Neighbor : Neighbors)
		{
			// skip if already visited
			if (Visited.Find(Neighbor) || MarkedForPropagation.Contains(Neighbor))
				continue;

			const FInfNode* NeighborNode = InfluenceMap->Find(Neighbor);
			
			//check node in propagation range
			float CenterNeighborDistSq = FVector::DistSquared(NeighborNode->GetNodeLocation(), CentreLocation);
			if (CenterNeighborDistSq <= MaxRangeSquared)
			{
				CacheNeighbors.Emplace(NeighborNode->GetGraphLocation(), CurrentNode.Value + 1);
				NeedPropagationCuration.Emplace(NeighborNode->GetGraphLocation(), CurrentNode.Value + 1);
				MarkedForPropagation.Emplace(Neighbor);
				MaxStep = FMath::Max(MaxStep, CurrentNode.Value + 1);
			}
		}

		if(WorkingBuffer.Num() <1)
		{
			WorkingBuffer.Append(CacheNeighbors);
			CacheNeighbors.Reset();
		}
		//TODO: Debug this maybe?
	}

	//iterate through the curation list, assign their value base on distance/curve/falloff...etc
	for (const auto& Pair : NeedPropagationCuration)
	{
		// get value from lambda
		Result.Emplace(Pair.Key, PropagationValueFunc(Pair.Value, MaxStep, InfluenceMap->Find(Pair.Key)->GetNodeLocation()));
	}

	return Result;
}

const TMap<FIntVector, float>& UInfPropagator::CreateInterestMap(float InterestMapRadius, const FVector& Center,
	UCurveFloat* InterestCurve, float InitializeValue)
{
	//if far away or already have interest map, skip
	if (!ShouldUpdatePropagationMap() && RecentInterestMap.Num() != 0)
	{
		return RecentInterestMap;
	}

	//this function is for EQS, CentreNode is the QueryContext
	//we get it offset position from the propagator 
	FVector Offset = Center - GetOwner()->GetActorLocation();
	FVector FeetLocation = GetOwner()->GetActorLocation() + OwnerFeetOffset + Offset;
	const FInfNode* FeetNode = InfluenceMapCollectionRef->GetNodeGraph()->FindNearestNode(FeetLocation);
	if (FeetNode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateAndInitMap() : Failed to find nearest node"));
		return RecentInterestMap;
	}

	auto PropagationCalculator = [&](uint32 Distance, uint32 MaxDistance, const FVector& NodeLocation)
	{
		if (PropagationCurve)
		{
			float DistSq = FVector::DistSquared(FeetLocation, NodeLocation);
			return PropagationCurve->GetFloatValue(DistSq / FMath::Square(InterestMapRadius));
		}

		return InitializeValue;
	};

	auto ExcludeFromPropagationCalc = [&](const AActor* TargetNodeActor)
	{
		return true;
	};

	RecentInterestMap = CreateNewMap(FeetNode, InterestMapRadius, PropagationCalculator, ExcludeFromPropagationCalc);

	return RecentInterestMap;
}

FVector UInfPropagator::GetGraphLocationToWorld(const FIntVector& GraphLocation)
{
	const auto* NodeGraph = InfluenceMapCollectionRef->GetNodeGraph();
	if (NodeGraph == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::GetGraphLocationToWorld : Invalid node graph."));
		return FVector();
	}

	const auto* Node = NodeGraph->GetNode(GraphLocation);
	if (Node == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::GetGraphLocationToWorld : Invalid graph location %s."), *GraphLocation.ToString());
		return FVector();
	}

	return Node->GetNodeLocation();
}

TMap<FIntVector, float> UInfPropagator::GetEnemyMap(const FGameplayTag MapTag, float GatherDistance) const
{
	const auto* TargetMap = InfluenceMapCollectionRef->GetMapSafe(MapTag);
	if (TargetMap == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::GetEnemyMap : Invalid map name. %s"), *MapTag.ToString());
		return TMap<FIntVector, float>();
	}
		
	
	return TargetMap->GatherTeamMap(EnemyTeam, this, false, GatherDistance);
}

TMap<FIntVector, float> UInfPropagator::GetAllyMap(const FGameplayTag MapTag, bool bIgnoreSelf, float GatherDistance) const
{
	const auto* TargetMap = InfluenceMapCollectionRef->GetMapSafe(MapTag);
	if (TargetMap == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInfluencePropagator::GetEnemyMap : Invalid map name. %s"), *MapTag.ToString());
		return TMap<FIntVector, float>();
	}

	return TargetMap->GatherTeamMap(AllyTeam, this, bIgnoreSelf, GatherDistance);
}

//TODO: this method just draw every map right now
void UInfPropagator::DrawDebugPropagationMap(const FGameplayTag MapTag, float Duration) const
{
	const IInfGraphInterface* NodeGraph = InfluenceMapCollectionRef->GetNodeGraph();
	if (NodeGraph == nullptr)
		return;
	FVector Offset(0,0, 100);
	for (const FPropagationMap& PropStamp : PropagationMapHistory)
	{
		for (const auto& Pair : PropStamp.MapData)
		{
			const FInfNode* Node = NodeGraph->GetNode(Pair.Key);
			DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + Offset, 16.f, UInfMapFunctionLibrary::ConvertInfluenceValueToColor(Pair.Value), false, Duration);
			DrawDebugString(GetWorld(), Node->GetNodeLocation(), FString::SanitizeFloat(Pair.Value), nullptr, FColor::White, Duration);
		}
	}
}

void UInfPropagator::DrawDebugWorkingMap(const TMap<FIntVector, float>& Map, float Duration)
{
	const IInfGraphInterface* NodeGraph = InfluenceMapCollectionRef->GetNodeGraph();
	if (NodeGraph == nullptr)
		return;
	
	FVector Offset(0,0, 100);
	for (const auto& Pair : Map)
	{
		const FInfNode* Node = NodeGraph->GetNode(Pair.Key);
		DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + Offset, 16.f, UInfMapFunctionLibrary::ConvertInfluenceValueToColor(Pair.Value), false, Duration);
		DrawDebugString(GetWorld(), Node->GetNodeLocation(), FString::SanitizeFloat(Pair.Value), nullptr, FColor::White, Duration);
	}
}

