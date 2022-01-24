// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/RecastNavMesh.h"
#include "InfNavMesh.generated.h"

/**
 * 
 */

UCLASS()
class ART_API AInfNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()
	AInfNavMesh(const FObjectInitializer& ObjectInitializer);
	static FPathFindingResult FindPath(const FNavAgentProperties& AgentProperties, const FPathFindingQuery& Query);
};
