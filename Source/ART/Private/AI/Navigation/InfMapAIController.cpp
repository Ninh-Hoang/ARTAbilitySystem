// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMapAIController.h"

#include "NavigationSystem.h"
#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AI/Navigation/InfNavMesh.h"
#include "VisualLogger/VisualLogger.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAIInfluenceNavigation, Log, All);
DEFINE_LOG_CATEGORY(LogAIInfluenceNavigation);

AInfMapAIController::AInfMapAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

AInfMapAIController::AInfMapAIController(AAIController& Controller)
{
}

FPathFollowingRequestResult AInfMapAIController::MoveToUseInfluenceMap(const FAIInfMapMoveRequest& MoveRequest,
	FNavPathSharedPtr* OutPath)
{
	UE_VLOG(this, LogAIInfluenceNavigation, Log, TEXT("MoveToUseInfluenceMap: %s"), *MoveRequest.ToString());

	FPathFollowingRequestResult ResultData;
	ResultData.Code = EPathFollowingRequestResult::Failed;

	if (MoveRequest.IsValid() == false)
	{
		UE_VLOG(this, LogAIInfluenceNavigation, Error, TEXT("MoveToUseInfluenceMap request failed due MoveRequest not being valid. Most probably desireg Goal Actor not longer exists"), *MoveRequest.ToString());
		return ResultData;
	}

	if (GetPathFollowingComponent() == nullptr)
	{
		UE_VLOG(this, LogAIInfluenceNavigation, Error, TEXT("MoveToUseInfluenceMap request failed due missing PathFollowingComponent"));
		return ResultData;
	}

	ensure(MoveRequest.GetNavigationFilter() || !DefaultNavigationFilterClass);

	bool bCanRequestMove = true;
	bool bAlreadyAtGoal = false;
	
	if (!MoveRequest.IsMoveToActorRequest())
	{
		if (MoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
		{
			UE_VLOG(this, LogAIInfluenceNavigation, Error, TEXT("AInfluenceMapAIController::MoveToUseInfluenceMap: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			bCanRequestMove = false;
		}

		// fail if projection to navigation is required but it failed
		if (bCanRequestMove && MoveRequest.IsProjectingGoal())
		{
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			const FNavAgentProperties& AgentProps = GetNavAgentPropertiesRef();
			FNavLocation ProjectedLocation;

			if (NavSys && !NavSys->ProjectPointToNavigation(MoveRequest.GetGoalLocation(), ProjectedLocation, INVALID_NAVEXTENT, &AgentProps))
			{
				if (MoveRequest.IsUsingPathfinding())
				{
					UE_VLOG_LOCATION(this, LogAIInfluenceNavigation, Error, MoveRequest.GetGoalLocation(), 30.f, FColor::Red, TEXT("AInfluenceMapAIController::MoveToUseInfluenceMap failed to project destination location to navmesh"));
				}
				else
				{
					UE_VLOG_LOCATION(this, LogAIInfluenceNavigation, Error, MoveRequest.GetGoalLocation(), 30.f, FColor::Red, TEXT("AInfluenceMapAIController::MoveToUseInfluenceMap failed to project destination location to navmesh, path finding is disabled perhaps disable goal projection ?"));
				}

				bCanRequestMove = false;
			}

			MoveRequest.UpdateGoalLocation(ProjectedLocation.Location);
		}

		bAlreadyAtGoal = bCanRequestMove && GetPathFollowingComponent()->HasReached(MoveRequest);
	}
	else
	{
		bAlreadyAtGoal = bCanRequestMove && GetPathFollowingComponent()->HasReached(MoveRequest);
	}

	if (bAlreadyAtGoal)
	{
		UE_VLOG(this, LogAIInfluenceNavigation, Log, TEXT("MoveToUseInfluenceMap: already at goal!"));
		ResultData.MoveId = GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		ResultData.Code = EPathFollowingRequestResult::AlreadyAtGoal;
	}
	else if (bCanRequestMove)
	{
		FPathFindingQuery PFQuery;

		const bool bValidQuery = BuildPathfindingQuery(MoveRequest, PFQuery);
		if (bValidQuery)
		{
			FNavPathSharedPtr Path;
			FindPathForInfluenceMapMoveRequest(MoveRequest, PFQuery, Path);

			const FAIRequestID RequestID = Path.IsValid() ? RequestMove(MoveRequest, Path) : FAIRequestID::InvalidRequest;
			if (RequestID.IsValid())
			{
				bAllowStrafe = MoveRequest.CanStrafe();
				ResultData.MoveId = RequestID;
				ResultData.Code = EPathFollowingRequestResult::RequestSuccessful;

				if (OutPath)
				{
					*OutPath = Path;
				}
			}
		}
	}

	if (ResultData.Code == EPathFollowingRequestResult::Failed)
	{
		ResultData.MoveId = GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
	}

	return ResultData;
}

void AInfMapAIController::FindPathForInfluenceMapMoveRequest(const FAIInfMapMoveRequest& MoveRequest,
	FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const
{
	SCOPE_CYCLE_COUNTER(STAT_AI_Overall);

	UE_LOG(LogTemp, Warning, TEXT("AInfluenceMapAIController::FindPathForMoveRequest"));
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		bool bSuccessGetNavmesh = false;
		
		if (AInfNavMesh* NavMesh = UInfMapFunctionLibrary::GetInfNavMesh(GetWorld()))
		{
			NavMesh->SetInfluenceMapData(MoveRequest);
			
			Query.NavData = NavMesh;
			Query.QueryFilter = NavMesh->GetDefaultQueryFilter();

			bSuccessGetNavmesh = true;
		}
		
		if (bSuccessGetNavmesh)
		{
			FPathFindingResult PathResult = NavSys->FindPathSync(Query);
			if (PathResult.Result != ENavigationQueryResult::Error)
			{
				if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
				{
					if (MoveRequest.IsMoveToActorRequest())
					{
						PathResult.Path->SetGoalActorObservation(*MoveRequest.GetGoalActor(), 100.0f);
					}

					PathResult.Path->EnableRecalculationOnInvalidation(true);
					OutPath = PathResult.Path;
				}
			}
			else
			{
				UE_VLOG(this, LogAIInfluenceNavigation, Error, TEXT("Trying to find path to %s resulted in Error")
						, MoveRequest.IsMoveToActorRequest() ? *GetNameSafe(MoveRequest.GetGoalActor()) : *MoveRequest.GetGoalLocation().ToString());
				UE_VLOG_SEGMENT(this, LogAIInfluenceNavigation, Error, GetPawn() ? GetPawn()->GetActorLocation() : FAISystem::InvalidLocation
								, MoveRequest.GetGoalLocation(), FColor::Red, TEXT("Failed move to %s"), *GetNameSafe(MoveRequest.GetGoalActor()));
			}
		}

	}
}

void AInfMapAIController::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	Super::GrabDebugSnapshot(Snapshot);
}
