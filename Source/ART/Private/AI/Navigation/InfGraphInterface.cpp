// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfGraphInterface.h"


// Add default functionality here for any IInfGraphInterface functions that are not pure virtual.
FInfNode* IInfGraphInterface::FindNearestNode(const FVector& FeetLocation)
{
	return nullptr;
}

FInfMap* IInfGraphInterface::GetNodeGraphData()
{
	return nullptr;
}

FInfNode* IInfGraphInterface::GetNode(const FIntVector& Key)
{
	return nullptr;
}
