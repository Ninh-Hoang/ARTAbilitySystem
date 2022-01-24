// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Navigation/InfNavMesh.h"

DECLARE_CYCLE_STAT(TEXT("Custom Pathfinding"), STAT_Navigation_CustomPathfinding, STATGROUP_Navigation)

AInfNavMesh::AInfNavMesh(const FObjectInitializer& ObjectInitializer)
{
	FindPathImplementation = FindPath;
}

FPathFindingResult AInfNavMesh::	FindPath(const FNavAgentProperties& AgentProperties, const FPathFindingQuery& Query)
{
	//declare our own performance log
	SCOPE_CYCLE_COUNTER(STAT_Navigation_CustomPathfinding);

	const ANavigationData* Self = Query.NavData.Get();
	check(Cast<const AInfNavMesh>(Self));
 
	const ARecastNavMesh* RecastNavMesh = (const ARecastNavMesh*)Self;
	if (Self == NULL)
	{
		return ENavigationQueryResult::Error;
	}
 
	FPathFindingResult Result(ENavigationQueryResult::Error);
 
	FNavigationPath* NavPath = Query.PathInstanceToFill.Get();
	FNavMeshPath* NavMeshPath = NavPath ? NavPath->CastPath<FNavMeshPath>()  : nullptr;
 
	if (NavMeshPath)
	{
		Result.Path = Query.PathInstanceToFill;
		NavMeshPath->ResetForRepath();
	}
	else
	{
		Result.Path = Self->CreatePathInstance<FNavMeshPath>(Query);
		NavPath = Result.Path.Get();
		NavMeshPath = NavPath ? NavPath->CastPath<FNavMeshPath>() : nullptr;
	}
 
	const FNavigationQueryFilter* NavFilter = Query.QueryFilter.Get();
	if (NavMeshPath && NavFilter)
	{
		NavMeshPath->ApplyFlags(Query.NavDataFlags);
 
		const FVector AdjustedEndLocation = NavFilter->GetAdjustedEndLocation(Query.EndLocation);
		if ((Query.StartLocation - AdjustedEndLocation).IsNearlyZero() == true)
		{
			Result.Path->GetPathPoints().Reset();
			Result.Path->GetPathPoints().Add(FNavPathPoint(AdjustedEndLocation));
			Result.Result = ENavigationQueryResult::Success;
		}
		else
		{
			//custom pathfinding
			
			const bool bPartialPath = Result.IsPartial();
			if (bPartialPath)
			{
				Result.Result = Query.bAllowPartialPaths ? ENavigationQueryResult::Success : ENavigationQueryResult::Fail;
			}
		}
	}
 
	return Result;
}
	