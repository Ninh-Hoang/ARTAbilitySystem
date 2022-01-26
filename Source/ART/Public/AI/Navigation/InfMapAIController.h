// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITask_InfMapMoveTo.h"
#include "InfMapAIController.generated.h"

UCLASS()
class ART_API AInfMapAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AInfMapAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	AInfMapAIController(AAIController& Controller);
	
	FPathFollowingRequestResult MoveToUseInfluenceMap(const FAIInfMapMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr);
	
	void FindPathForInfluenceMapMoveRequest(const FAIInfMapMoveRequest& MoveRequest, FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const;

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif
};
