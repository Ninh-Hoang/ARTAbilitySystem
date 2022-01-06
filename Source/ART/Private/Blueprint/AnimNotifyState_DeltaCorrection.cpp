// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/AnimNotifyState_DeltaCorrection.h"

#include "Blueprint/ARTBlueprintFunctionLibrary.h"

void UAnimNotifyState_DeltaCorrection::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                   float TotalDuration)
{
	FGameplayEventData Payload;
	Payload.EventTag = CorrectionEventTag;
	Payload.EventMagnitude = TotalDuration;
	UARTBlueprintFunctionLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), CorrectionEventTag, Payload);
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
}
