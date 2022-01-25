// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "InfCollectionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInfCollectionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IInfCollectionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual class IInfMapInterface* GetMapSafe(const FGameplayTag& MapTag) const = 0;
	virtual const class IInfGraphInterface* GetNodeGraph() const = 0;
};
