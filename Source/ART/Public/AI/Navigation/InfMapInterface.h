// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "InfMapInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInfMapInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInfMapInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Initialize(class IInfCollectionInterface* MapCollectionInterface) = 0;
	virtual void AddPropagator(class IInfPropagatorInterface* NewPropagator) = 0;
	virtual void RemovePropagator(class IInfPropagatorInterface* NewPropagator) = 0;
	virtual const TArray<class IInfPropagatorInterface*>& GetPropagators() const = 0;
	virtual void GatherMap(const FGameplayTagContainer& BehaviourTags,
		const FGameplayTagContainer& RequiredTags,
		const FGameplayTagContainer& BlockTags,
		const class IInfPropagatorInterface* Self,
		bool bIgnoreSelf,
		float GatherDistance,
		TMap<FIntVector, float>& Result) const = 0;
	virtual const TArray<uint32> GetAffectedTile() const = 0;
};
