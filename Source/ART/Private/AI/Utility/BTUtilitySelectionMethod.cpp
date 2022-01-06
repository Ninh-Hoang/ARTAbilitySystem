// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Utility/BTUtilitySelectionMethod.h"

int32 UBTUtilitySelectionMethod::SelectOption(TArray< float > const& UtilityValues)
{
	return NoSelection;
}

void UBTUtilitySelectionMethod::GenerateOrdering(TArray< float > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering)
{

}
