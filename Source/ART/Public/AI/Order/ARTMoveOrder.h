// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacterAIOrder.h"
#include "ARTMoveOrder.generated.h"

class AActor;
struct FARTOrderTargetData;

/**
* Standard unit movement to a fixed target location on the map. Calculates a formation for multiple units using their
* formation rank.
*/
UCLASS(NotBlueprintType, Abstract, Blueprintable)
class ART_API UARTMoveOrder : public UARTCharacterAIOrder
{
	GENERATED_BODY()

public:
	UARTMoveOrder();

	//~ Begin UARTOrder Interface
	virtual void CreateIndividualTargetLocations(const TArray<AActor*>& OrderedActors,
												const FARTOrderTargetData& TargetData,
												TArray<FVector2D>& OutTargetLocations) const override;
	//~ End UARTOrder Interface

private:
	/**
	* Creates a block formation of unit locations.
	* @param UnitCount The required amount of locations.
	* @param Direction The direction the formation should face to. Does not have to be normalized.
	* @param TargetLocation Target location of the formation.
	* @param OutLocations An array that will be filled with the locations starting from the front line.
	*/
	void CalculateFormation(int32 UnitCount, const FVector2D Direction, const FVector2D TargetLocation,
							TArray<FVector2D>& OutLocations) const;

	/** Gets the 2D center of the specified group of actors. */
	FVector2D GetCenterOfGroup(const TArray<AActor*>& Actors) const;

	/** Gets the formation rank of the specified unit. */
	int32 GetFormationRank(const AActor* Unit) const;
	
};
