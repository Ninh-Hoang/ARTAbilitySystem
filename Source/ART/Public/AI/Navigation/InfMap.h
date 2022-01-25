﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfMapInterface.h"
#include "GameFramework/Actor.h"
#include "InfMap.generated.h"

UCLASS()
class ART_API AInfMap : public AActor, public IInfMapInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInfMap();

	UPROPERTY(EditAnywhere, Category = "NodeGraph")
	bool bTickEnabled;

	UPROPERTY(EditAnywhere, Category = "NodeGraph", meta = (ClampMin = 0.10))
	float TickInterval;

	UPROPERTY(EditAnywhere, Category = "NodeGraph | DEBUG")
	bool bDrawDebugInfluenceMap;

private:
	TArray<class IInfPropagatorInterface*> Propagators;

	UPROPERTY()
	TScriptInterface<class IInfCollectionInterface> InfluenceMapCollectionInterfaceScript;
	
	class IInfCollectionInterface* InfluenceMapCollectionRef;

public:
	virtual void Initialize(class IInfCollectionInterface* MapCollectionInterface) override;
	virtual void AddPropagator(class IInfPropagatorInterface* NewPropagator) override { Propagators.Add(NewPropagator); }
	virtual const TArray<class IInfPropagatorInterface*>& GetPropagators() const override { return Propagators; }
	virtual TMap<FIntVector, float> GatherTeamMap(const TArray<FGenericTeamId>& Teams, const class IInfPropagatorInterface* Self, bool bIgnoreSelf = false, float GatherDistance = 0.f) const override;

private:
	void UpdateInfluenceMap();

	// ƒfƒoƒbƒO
	void MergePropgationStamp(const TArray<TMap<FIntVector, float>>& PropagationStamps, TMap<FIntVector, float>& CompleteStamp);
	void DrawDebugIMap(const TMap<FIntVector, float>& CompleteStamp, bool bDrawConnectingNeighbor) const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
