// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BTUtilityTypes.h"
#include "BTUtilitySelectionMethod.generated.h"

/** 
* A utility selection method chooses from a list of options each of which has an associated utility value.
*/
UCLASS()
class ART_API UBTUtilitySelectionMethod : public UObject
{
	GENERATED_BODY()

	public:
	static const int32 NoSelection = -1;

	public:
	virtual int32 SelectOption(TArray< float > const& UtilityValues);
	virtual void GenerateOrdering(TArray< float > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering);
};
