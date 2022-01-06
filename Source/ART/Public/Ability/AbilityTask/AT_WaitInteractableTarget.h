// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "AT_WaitInteractableTarget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitInteractableTargetDelegate, const FGameplayAbilityTargetDataHandle&,
                                            Data);

UCLASS()
class ART_API UAT_WaitInteractableTarget : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate FoundNewInteractableTarget;

	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate LostInteractableTarget;

	/**
	* Traces a line on a timer looking for InteractableTargets.
	* @param MaxRange How far to trace.
	* @param TimerPeriod Period of trace timer.
	* @param bShowDebug Draws debug lines for traces.
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",
		BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAT_WaitInteractableTarget* WaitForInteractableTarget(UGameplayAbility* OwningAbility,
	                                                             FName TaskInstanceName,
	                                                             FCollisionProfileName TraceProfile,
	                                                             float MaxRange = 200.0f, float TimerPeriod = 0.1f,
	                                                             bool bTraceWithCursor = false, bool bShowDebug = true);

	virtual void Activate() override;

protected:
	FGameplayAbilityTargetingLocationInfo StartLocation;


	float MaxRange;

	float TimerPeriod;

	bool bTraceWithCursor;

	bool bShowDebug;

	FCollisionProfileName TraceProfile;

	FGameplayAbilityTargetDataHandle TargetData;

	FTimerHandle TraceTimerHandle;

	virtual void OnDestroy(bool AbilityEnded) override;

	/** Traces as normal, but will manually filter all hit actors */
	void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End,
	               FName ProfileName, const FCollisionQueryParams Params, bool bLookForInteractableActor) const;

	void AimWithPlayerPawn(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart,
	                       FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	//bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition) const;

	UFUNCTION()
	void PerformTrace();

	FGameplayAbilityTargetDataHandle MakeTargetData(const FHitResult& HitResult) const;
};
