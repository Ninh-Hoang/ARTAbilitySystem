// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMap.h"

#include "DrawDebugHelpers.h"
#include "AI/Navigation/InfCollectionInterface.h"
#include "AI/Navigation/InfGraphInterface.h"
#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AI/Navigation/InfPropagatorInterface.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"


// Sets default values
AInfMap::AInfMap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bTickEnabled = true;

	//for now map only update at 1Hz
	TickInterval = 1.0f;
	bDrawDebugInfluenceMap = true;
}

void AInfMap::Initialize(IInfCollectionInterface* MapCollectionInterface)
{
	//cache the collection interface 
	InfluenceMapCollectionRef = MapCollectionInterface;
	InfluenceMapCollectionInterfaceScript.SetObject(MapCollectionInterface->_getUObject());
}

// Called when the game starts or when spawned
void AInfMap::BeginPlay()
{
	Super::BeginPlay();

	if (InfluenceMapCollectionRef == nullptr)
		InfluenceMapCollectionRef = Cast<IInfCollectionInterface>(InfluenceMapCollectionInterfaceScript.GetObject());

	SetActorTickInterval(TickInterval);
}

// Called every frame
void AInfMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bTickEnabled) UpdateInfluenceMap();
}

void AInfMap::UpdateInfluenceMap()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AInfMap_UpdateInfluenceMap);

	if (Propagators.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AInfluenceMap::UpdateInfluenceMap() : Propagators is empty. Place an actor with at least one Influence Propagator Component attached to the level."));
		return;
	}

	// update map by calling all propagation interface
	ParallelFor(Propagators.Num(), [&](int32 PropIdx) { Propagators[PropIdx]->UpdatePropagationMap(); });
	
	if (bDrawDebugInfluenceMap)
	{
		TArray<TMap<FIntVector, float>> PropagationStamps;
		PropagationStamps.SetNum(Propagators.Num());

		//collect all map from propagator
		ParallelFor(Propagators.Num(), [&](int32 PropIdx)
					{
						PropagationStamps[PropIdx] = Propagators[PropIdx]->GetPropagationMap();
					});
		TMap<FIntVector, float> CompleteStamp;
		CompleteStamp.Reserve(InfluenceMapCollectionRef->GetNodeGraph()->GetNodeGraphData()->NodeMap.Num());

		//merge map together
		MergePropgationStamp(PropagationStamps, CompleteStamp);
		DrawDebugIMap(CompleteStamp, false);
	}
}

void AInfMap::MergePropgationStamp(const TArray<TMap<FIntVector, float>>& PropagationStamps,
	TMap<FIntVector, float>& CompleteStamp)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AInfMap_MergePropgationStamp);

	for (const auto& PropagationStamp : PropagationStamps)
	{
		for (const auto& MapData : PropagationStamp)
		{
			if (CompleteStamp.Contains(MapData.Key))
				CompleteStamp[MapData.Key] += MapData.Value;
			else
				CompleteStamp.Add(MapData);
		}
	}
}

void AInfMap::DrawDebugIMap(const TMap<FIntVector, float>& CompleteStamp, bool bDrawConnectingNeighbor) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AInfMap_DrawDebugIMap);

	if (CompleteStamp.Num() == 0)
		return;

	const FInfMap* BaseMap = InfluenceMapCollectionRef->GetNodeGraph()->GetNodeGraphData();
	const TMap<FIntVector, FInfNode>* NodeGraph = &BaseMap->NodeMap;
	
	float Duration = TickInterval;
	FVector Offset(0,0, 100);
	for (const auto& Data : CompleteStamp)
	{
		const FInfNode* Node = NodeGraph->Find(Data.Key);

		//TODO: this is need for when mesh to rebuild, might be able to remove when we get the regeneration done
		if(!Node) continue;
		DrawDebugPoint(GetWorld(), Node->GetNodeLocation() + Offset, 16.f, UInfMapFunctionLibrary::ConvertInfluenceValueToColor(Data.Value), false, Duration);
		DrawDebugString(GetWorld(), Node->GetNodeLocation(), FString::SanitizeFloat(Data.Value), nullptr, FColor::White, Duration);
		if (bDrawConnectingNeighbor)
		{
			for (const FIntVector NeighborID : Node->GetNeighbor())
			{
				const FInfNode* Neighbor = NodeGraph->Find(NeighborID);
				FVector LineEnd = Node->GetNodeLocation() + (Neighbor->GetNodeLocation() - Node->GetNodeLocation()) * 0.4f;
				DrawDebugDirectionalArrow(GetWorld(), Node->GetNodeLocation() + Offset, LineEnd + Offset, 50.f, FColor::Green, true, Duration, 0, 1.5f);
			}
		}
	}
}

void AInfMap::GatherMap(const FGameplayTagContainer& BehaviourTags,
	const FGameplayTagContainer& RequiredTags,
	const FGameplayTagContainer& BlockTags,
	const IInfPropagatorInterface* Self,
	bool bIgnoreSelf,
	float GatherDistance,
	TMap<FIntVector, float>& Result) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AInfMapCollection_GatherMap);

	bool NeedToCheckBehaviourTags = !BehaviourTags.IsEmpty();
	bool NeedToCheckRequiredTags = !RequiredTags.IsEmpty();
	bool NeedToCheckBlockedTags = !BlockTags.IsEmpty();
	
	Result.Reserve(InfluenceMapCollectionRef->GetNodeGraph()->GetNodeGraphData()->NodeMap.Num());

	const FVector& SelfLocation = Self->GetOwnerActor()->GetActorLocation();
	bool bInfiniteGatherDistance = (GatherDistance <= 0.f);
	float ThresholdSq = (bInfiniteGatherDistance) ? FLT_MAX : FMath::Square(GatherDistance);
	
	for (const IInfPropagatorInterface* Propagator : GetPropagators())
	{
		if (bIgnoreSelf && Propagator == Self)	continue;

		if (!bInfiniteGatherDistance && FVector::DistSquared(SelfLocation, Propagator->GetOwnerActor()->GetActorLocation()) > ThresholdSq) continue;
		
		if (NeedToCheckBehaviourTags)
		{
			FGameplayTagContainer AttitudeTag;
			UARTBlueprintFunctionLibrary::GetTeamAttitudeTags(Self->GetOwnerActor(), Propagator->GetOwnerActor(), AttitudeTag);
			if(!AttitudeTag.HasAll(BehaviourTags)) continue;
		}

		if(NeedToCheckBlockedTags || NeedToCheckRequiredTags)
		{
			auto ASC = Cast<IAbilitySystemInterface>(Propagator->GetOwnerActor())->GetAbilitySystemComponent();
			
			FGameplayTagContainer PropagatorTags;
			UARTBlueprintFunctionLibrary::GetTags(Propagator->GetOwnerActor(), PropagatorTags);
			if(!UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(PropagatorTags, RequiredTags, BlockTags)) continue;
		}

		for (const auto& Pair : Propagator->GetPropagationMap())
		{
			float Val = Result.FindOrAdd(Pair.Key, Pair.Value);
			
			if (Val != Pair.Value)
				Result[Pair.Key] += Pair.Value;
		}
	}
}

const TArray<uint32> AInfMap::GetAffectedTile() const
{
	TArray<uint32> AffectedTiles;
	
	for(auto& Propagator : Propagators)
	{
		const TArray<uint32>& PropagatorAffectedTile = Propagator->GetAffectedTile();
		for(uint32 TileIndex : PropagatorAffectedTile)
		{
			AffectedTiles.AddUnique(TileIndex);
		}
	}
	return AffectedTiles;
}
