// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "AITypes.h"
#include "InfMapFunctionLibrary.h"
#include "Tasks/AITask.h"
#include "Navigation/PathFollowingComponent.h"
#include "AITask_InfMapMoveTo.generated.h"

/**
 * 
 */
USTRUCT()
struct ART_API FAIInfMapMoveRequest : public FAIMoveRequest
{
	GENERATED_USTRUCT_BODY()
	FAIInfMapMoveRequest();
	FAIInfMapMoveRequest(const AActor* InGoalActor);
	FAIInfMapMoveRequest(const FVector& InGoalLocation);
	FAIInfMapMoveRequest(FAIMoveRequest& BaseMoveRequest);

	FAIMoveRequest& SetInfluenceQueryData(const FInfQueryData& MapQueryData)
	{
		InfluenceQueryData = &MapQueryData;
		return *this;
	}
	FAIMoveRequest& SetCostMultiplier(float Multiplier)
	{
		CostMultiplier = Multiplier;
		return *this;
	}

	const FInfQueryData* GetInfluenceQueryData() const { return InfluenceQueryData; }
	float GetCostMultiplier() const { return CostMultiplier; }

protected:
	const FInfQueryData* InfluenceQueryData;
	float CostMultiplier;
};

class AInfMapAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInfMapMoveTaskCompletedSignature, TEnumAsByte<EPathFollowingResult::Type>, Result, AAIController*, AIController);

UCLASS()
class ART_API UAITask_InfMapMoveTo : public UAITask
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	FAIInfMapMoveRequest InfluenceMapMoveRequest;

	UPROPERTY(BlueprintReadOnly, Category="AI|Tasks")
	TObjectPtr<AInfMapAIController> OwnerInfluenceMapAIController;

public:
	UAITask_InfMapMoveTo(const FObjectInitializer& ObjectInitializer);

	/** tries to start move request and handles retry timer */
	void ConditionalPerformMove();

	/** prepare move task for activation */
	void SetUp(AAIController* Controller, const FAIMoveRequest& InMoveRequest);

	EPathFollowingResult::Type GetMoveResult() const { return MoveResult; }
	bool WasMoveSuccessful() const { return MoveResult == EPathFollowingResult::Success; }
	bool WasMovePartial() const { return Path.IsValid() && Path->IsPartial(); }

	void SetInfluenceMapMoveRequest(const FAIInfMapMoveRequest& InRequest);
	
	UFUNCTION(BlueprintCallable, Category = "AI|Tasks", meta = (AdvancedDisplay = "AcceptanceRadius,StopOnOverlap,AcceptPartialPath,bUsePathfinding,bUseContinuosGoalTracking,ProjectGoalOnNavigation", DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE", DisplayName = "Move To Location or Actor Use Influence Map Data"))
	static UAITask_InfMapMoveTo* AIMoveToUseInfluenceMapData(class AAIController* Controller,
																	   const FInfQueryData& InfluenceQueryData,
																	   float CostMultiplier = 1.0f,
																	   FVector GoalLocation = FVector(0.f),
																	   AActor* GoalActor = nullptr,
																	   float AcceptanceRadius = -1.f,
																	   EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default,
																	   EAIOptionFlag::Type AcceptPartialPath = EAIOptionFlag::Default,
																	   bool bUsePathfinding = true,
																	   bool bLockAILogic = false,
																	   bool bUseContinuosGoalTracking = false,
																	   EAIOptionFlag::Type ProjectGoalOnNavigation = EAIOptionFlag::Default);


/** Allows custom move request tweaking. Note that all MoveRequest need to 
	 *	be performed before PerformMove is called. */
	FAIMoveRequest& GetMoveRequestRef() { return MoveRequest; }

	/** Switch task into continuous tracking mode: keep restarting move toward goal actor. Only pathfinding failure or external cancel will be able to stop this task. */
	void SetContinuousGoalTracking(bool bEnable);

protected:
	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnRequestFailed;

	UPROPERTY(BlueprintAssignable)
	FInfMapMoveTaskCompletedSignature OnMoveFinished;

	/** parameters of move request */
	UPROPERTY()
	FAIMoveRequest MoveRequest;

	/** handle of path following's OnMoveFinished delegate */
	FDelegateHandle PathFinishDelegateHandle;

	/** handle of path's update event delegate */
	FDelegateHandle PathUpdateDelegateHandle;

	/** handle of active ConditionalPerformMove timer  */
	FTimerHandle MoveRetryTimerHandle;

	/** handle of active ConditionalUpdatePath timer */
	FTimerHandle PathRetryTimerHandle;

	/** request ID of path following's request */
	FAIRequestID MoveRequestID;

	/** currently followed path */
	FNavPathSharedPtr Path;

	TEnumAsByte<EPathFollowingResult::Type> MoveResult;
	uint8 bUseContinuousTracking : 1;

	virtual void Activate() override;
	virtual void OnDestroy(bool bOwnerFinished) override;

	virtual void Pause() override;
	virtual void Resume() override;

	/** finish task */
	void FinishMoveTask(EPathFollowingResult::Type InResult);

	/** stores path and starts observing its events */
	void SetObservedPath(FNavPathSharedPtr InPath);

	/** remove all delegates */
	virtual void ResetObservers();

	/** remove all timers */
	virtual void ResetTimers();

	/** tries to update invalidated path and handles retry timer */
	void ConditionalUpdatePath();

	/** start move request */
	virtual void PerformMove();

	/** event from followed path */
	virtual void OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event);

	/** event from path following */
	virtual void OnRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
};
