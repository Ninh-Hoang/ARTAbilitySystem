// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ARTInventoryInterface.generated.h"

class UARTInventoryComponent;

// This class does not need to be modified.
UINTERFACE()
class UARTInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ART_API IARTInventoryInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual UARTInventoryComponent* GetInventoryComponent() const = 0;
};
