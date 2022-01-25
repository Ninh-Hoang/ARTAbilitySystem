// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfPropagator.h"
#include "AI/Navigation/InfCollectionInterface.h"
#include "AI/Navigation/InfMapInterface.h"
#include "AI/Navigation/InfGraphInterface.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UInfPropagator::UInfPropagator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	/*bTickEnabled = true;
	PropagateRange = 500.f;
	AttenuationRatio = 1.f;*/
}
/*
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

	//if not activated skip
	if (!bTickEnabled)
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

	if (FeetNode == nullptr || FeetNode->GetID() == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdatePropagationMap() : Failed to find nearest node. Need rebuild NodeGraph & Influence Map"));
		return;
	}

	//add recent propagation map to history if required, else, create a new map and add it
	if (PropagationMapHistory.Num() > 0 && (!ShouldUpdatePropagationMap() || FeetNode->GetID() == INDEX_NONE))
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
}

TMap<FIntVector, float> UInfPropagator::CreateNewPropagationMap(const FInfNode* CenterNode) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UInfPropagator_CreateNewPropagationMap);

	const auto* NodeGraphRef = InfluenceMapCollectionRef->GetNodeGraph();
	FHitResult TraceResult;
	
	auto PropagationCalculator = [&](uint32 StepDistance, uint32 MaxStepDistance, const FVector& NodeLocation)
	{
		// ƒŒƒCƒLƒƒƒXƒg‚µ‚¿‚á‚Á‚Ä‚é‚¯‚Ç‚³I
		GetWorld()->LineTraceSingleByChannel(TraceResult, 
											 GetOwner()->GetActorLocation() + OwnerHeadOffset, 
											 NodeLocation, 
											 ECollisionChannel::ECC_Visibility, // Pawn‚âCharacterMesh‚É‚Íƒqƒbƒg‚µ‚È‚¢ƒ`ƒƒƒ“ƒlƒ‹‚ðŽw’è.
											 FCollisionQueryParams{ "Propagate LOS", false, GetOwner() });

		//DrawDebugLine(GetWorld(), OwnerHeadLocation, NodeLocation, FColor::Red, false, 30.f);

		float Ratio = 0.f;
		// ‰½‚àÕ“Ë‚µ‚Ä‚¢‚È‚¢iŠÔ‚ÉáŠQ•¨‚ª‚È‚¢j ‚Ü‚½‚Í uNodevƒ^ƒO‚ðŽ‚ÂƒAƒNƒ^[‚Éƒqƒbƒg‚µ‚½
		if (!TraceResult.bBlockingHit || (TraceResult.GetActor() && TraceResult.GetActor()->ActorHasTag("Node")))
			Ratio = FVector::Dist(CenterNode->GetActorLocation(), NodeLocation) / PropagateRange;
		else
			Ratio = (float)StepDistance / (float)MaxStepDistance;

		if (PropagationCurve)
			return PropagationCurve->GetFloatValue(Ratio);
		else
			return FMath::Max(0.0f, 1.f - 1.f * Ratio);

	};

	auto ExcludeFromPropagationCalc = [&](const AActor* TargetNodeActor)
	{
		return true;
	};


	return CreateNewMap(CenterNode, PropagateRange, PropagationCalculator, ExcludeFromPropagationCalc);
}*/
