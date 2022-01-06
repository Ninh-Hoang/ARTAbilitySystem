// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ARTCrowdManager.h"

/*#if WITH_RECAST
#include "NavMesh/RecastHelpers.h"
#include "Navmesh/Public/Detour/DetourNavMesh.h"
#include "Navmesh/Public/DetourCrowd/DetourCrowd.h"
#endif*/

#include "Navigation/CrowdFollowingComponent.h"

UARTCrowdManager::UARTCrowdManager(const FObjectInitializer& ObjectInitializer)
{
}

/*void UARTCrowdManager::ApplyVelocity(UCrowdFollowingComponent* AgentComponent, int32 AgentIndex) const
{
	const dtCrowdAgent* ag = DetourCrowd->getAgent(AgentIndex);
	const dtCrowdAgentAnimation* anims = DetourCrowd->getAgentAnims();

	const FVector NewVelocity = Recast2UnrealPoint(ag->nvel);
	const float* RcDestCorner = anims[AgentIndex].active ? anims[AgentIndex].endPos : 
		ag->ncorners ? &ag->cornerVerts[0] : &ag->npos[0];

	const bool bIsNearEndOfPath = (ag->ncorners == 1) && ((ag->cornerFlags[0] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) == 0);

	const FVector DestPathCorner = Recast2UnrealPoint(RcDestCorner);
	AgentComponent->ApplyCrowdAgentVelocity(NewVelocity, DestPathCorner, anims[AgentIndex].active != 0, bIsNearEndOfPath);
}*/
