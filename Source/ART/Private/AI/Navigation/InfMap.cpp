// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMap.h"
#include "AI/Navigation/InfCollectionInterface.h"


// Sets default values
AInfMap::AInfMap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bTickEnabled = true;
	TickInterval = 1.0f;
	bDrawDebugInfluenceMap = true;
}

void AInfMap::Initialize(IInfCollectionInterface* MapCollectionInterface)
{
	InfluenceMapCollectionRef = MapCollectionInterface;
	InfluenceMapCollectionInterfaceScript.SetObject(MapCollectionInterface->_getUObject());
}

// Called when the game starts or when spawned
void AInfMap::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInfMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInfMap::UpdateInfluenceMap()
{
}

void AInfMap::MergePropgationStamp(const TArray<TMap<FIntVector, float>>& PropagationStamps,
	TMap<FIntVector, float>& CompleteStamp)
{
}

void AInfMap::DrawDebugIMap(const TMap<FIntVector, float>& CompleteStamp, bool bDrawConnectingNeighbor) const
{
}

TMap<FIntVector, float> AInfMap::GatherTeamMap(const TArray<int32>& Teams, const IInfPropagatorInterface* Self,
	bool bIgnoreSelf, float GatherDistance) const
{
	TMap<FIntVector, float> Temp;
	return Temp;
}