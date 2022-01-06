
#pragma once

#include "BTUtilityTypes.h"


namespace UtilitySelection
{
	/**
	* Chooses the node with the highest utility value.
	*/
	void PriorityOrdering(TArray< float > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering);
}