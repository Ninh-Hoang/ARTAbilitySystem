// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_MontageDeltaCorrection.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageDeltaCorrectionDelegate, int, CorrectionIndex);

UCLASS()
class ART_API UAT_MontageDeltaCorrection : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	/** 
        * ListenToEvent from AnimNotifyState for DeltaCorrection of Montage.
        * Location and Rotation correction are differed
        * Event are broadcast when correction start or end with their corresponding index, start from 0
        * Use External function for setting new target location/rotation in runtime for next correction
        * @param TaskInstanceName Set to override the name of this task, for later querying
        * @param Skeletal for finding anim instance and convert bone location to world
        * @param MontageToCorrect  The montage to find AnimInstance
        * @param LocationEventTags Tags of event that starts LocationCorrection
		* @param RotationEventTags Tags of event that starts RotationCorrection
        * @param CorrectTarget Target Vector for correction
        * @param CorrectRotationTarget Target to Rotate to
        * @param RotatePivotOffset offset the rotate pivot relative to actor rotation
        * @param ActorTarget Optional Actor for moving target
        * @param MontagePlayRate need for simulation calculation
        * @param RootMotionTranslationScale need for simulation calculation
        * @param ExpoAngle Angle of expo curve for ease in out interpolation
        * @param VelocityLimit Limit Correction Vec?
        * @param RotationSpeedLimit Limit Rotation Correction radian?
        * @param Velocity Max Speed
        * @param RotationSpeed Max Rotation Speed
        * @param DrawDebug Draw Debug?
        */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DisplayName="MontageDeltaCorrection",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_MontageDeltaCorrection* MontageDeltaCorrect(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		USkeletalMeshComponent* Skeletal,
		UAnimMontage* MontageToCorrect,
		FGameplayTagContainer LocationEventTags,
		FGameplayTagContainer RotationEventTags,
		FVector CorrectTarget = FVector(0, 0, 0),
		FVector CorrectRotationTarget = FVector(0, 0, 0),
		FRotator RotatePivotOffset = FRotator(0, 0, 0),
		AActor* ActorTarget = nullptr,
		float MontagePlayRate = 1.f,
		float RootMotionTranslationScale = 1.f,
		float ExpoAngle = 1.5f,
		bool VelocityLimit = false,
		bool RotationSpeedLimit = false,
		float Velocity = 0.f,
		float RotationSpeed = 0.f,
		bool DrawDebug = false);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/** Delta Correction Start or end */
	UPROPERTY(BlueprintAssignable)
	FMontageDeltaCorrectionDelegate OnLocationCorrectStart;

	UPROPERTY(BlueprintAssignable)
	FMontageDeltaCorrectionDelegate OnLocationCorrectEnd;

	UPROPERTY(BlueprintAssignable)
	FMontageDeltaCorrectionDelegate OnRotationCorrectStart;

	UPROPERTY(BlueprintAssignable)
	FMontageDeltaCorrectionDelegate OnRotationCorrectEnd;

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks")
	void ExternalSetCorrectionLocation(FVector InLocation, AActor* InActor);

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks")
	void ExternalSetCorrectionRotation(FVector InRotationTarget);

private:
	UPROPERTY()
	USkeletalMeshComponent* Skeletal;

	UPROPERTY()
	UAnimMontage* MontageToCorrect;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer LocationEventTags;

	UPROPERTY()
	FGameplayTagContainer RotationEventTags;

	UPROPERTY()
	FVector CorrectTarget;

	UPROPERTY()
	FVector CorrectRotationTarget;
	
	UPROPERTY()
	FRotator RotatePivotOffset;

	UPROPERTY()
	AActor* ActorTarget;

	UPROPERTY()
	float MontagePlayRate;

	UPROPERTY()
	float RootMotionTranslationScale;

	UPROPERTY()
	float ExpoAngle;

	UPROPERTY()
	bool VelocityLimit;

	UPROPERTY()
	bool RotationSpeedLimit;

	UPROPERTY()
	float Velocity;

	UPROPERTY()
	float RotationSpeed;

	UPROPERTY()
	bool DrawDebug;

	FGameplayTagContainer CombineEventTags;

	bool LocationDeltaCorrectionActivated;
	bool RotationDeltaCorrectionActivated;

	bool UseActorTarget;

	FVector OffSetVector;
	FRotator OffSetRotation;

	FVector RootStartLocation;
	FVector RootStartRotation;

	FVector RootEndLocation;
	
	FRotator ActorEndRotation;
	FVector ActorEndLocation;

	FVector CurrentLocationCorrection;
	FRotator CurrentRotationCorrection;

	float DeltaCorrectionTimeLocation;
	float RemainingDeltaTimeLocation;

	float DeltaCorrectionTimeRotation;
	float RemainingDeltaTimeRotation;

	float LocationAlpha;
	float RotationAlpha;

	float MontageStartTime;
	
	/* Handle to manage event */
	FDelegateHandle EventHandle;

	/* Handle to manage the timer */
	FTimerHandle LocationDeltaTimerHandle;
	FTimerHandle RotationDeltaTimerHandle;

	int LocationCorrectionIndex;
	int RotationCorrectionIndex;

	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);
	/** Returns our ability system component */
	class UARTAbilitySystemComponent* GetTargetASC();
	class UARTCharacterMovementComponent* GetTargetMovementComp();

	void DrawDeltaCorrectionDebug(FVector Location, float Radius, FColor Color, float Time);
	void EndLocationCorrection();
	void EndRotationCorrection();
};
