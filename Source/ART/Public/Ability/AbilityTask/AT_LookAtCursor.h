// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_LookAtCursor.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLookAtCursor);

UCLASS()
class ART_API UATLookAtCursor : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	// change the player controller rotation with LookAt rotation from pawn to cursor
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf =
		"OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UATLookAtCursor* LookAtCursor(UGameplayAbility* OwningAbility, FName TaskInstanceName,
	                                     class ACharacter* Player, class UCharacterMovementComponent* MovementComponent,
	                                     class APlayerController* PlayerController,
	                                     UCurveFloat* OptionalInterpolationCurve);

	virtual void Activate() override;

	// Tick function for this task, if bTickingTask == true
	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool AbilityIsEnding) override;

protected:
	bool bIsFinished;

	class UCharacterMovementComponent* MovementComponent;

	class APlayerController* PlayerController;

	class ACharacter* Player;

	float StartRotationRate;

	float RotationRate;

	UCurveFloat* LerpCurve;
};
