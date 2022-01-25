// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "InfPropagatorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInfPropagatorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInfPropagatorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Initialize(class IInfCollectionInterface* LayerMapCollection) = 0;
	virtual void UpdatePropagationMap() = 0;
	virtual const TMap<FIntVector, float>& GetPropagationMap() const = 0;
	virtual FGameplayTag GetTargetMapTag() const = 0;
	virtual int32 GetTeam() const = 0;
	virtual const AActor* GetOwnerActor() const = 0;
};
