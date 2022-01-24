// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfNode.h"
#include "UObject/Interface.h"
#include "InfNodeGraphInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInfNodeGraphInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInfNodeGraphInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FInfNode* FindNearestNode(const FVector& FeetLocation) const = 0;
	virtual const TMap<FIntVector, FInfNode*>& GetNodeGraphData()const = 0;
	virtual const FInfNode* GetNode(const FIntVector& Key) const = 0;
};
