// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DeltaCorrection.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UAnimNotifyState_DeltaCorrection : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	/* Event tag to send to DeltaCorrection GameplayTask */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AnimNotify)
	FGameplayTag CorrectionEventTag;

	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration);
	//virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime);
	//virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation);
};
