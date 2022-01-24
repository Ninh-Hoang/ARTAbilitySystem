// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfStruct.h"
#include "UObject/Interface.h"
#include "InfGraphInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInfGraphInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInfGraphInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FInfNode* FindNearestNode(const FVector& FeetLocation);
	virtual FInfMap* GetNodeGraphData();
	virtual FInfNode* GetNode(const FIntVector& Key);
};
