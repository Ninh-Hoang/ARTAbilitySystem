// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "AT_WaitAbilityConfirmCancel.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UAT_WaitAbilityConfirmCancel : public UAbilityTask_WaitConfirmCancel
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", DisplayName="Wait Ability Confirm/Cancel Input"), Category="Ability|Tasks")
	static UAbilityTask_WaitConfirmCancel* WaitAbilityConfirmCancel(UGameplayAbility* OwningAbility);

	virtual void Activate() override;

protected:
	FDelegateHandle DelegateHandleConfirm;
	FDelegateHandle DelegateHandleCancel;

	virtual void OnDestroy(bool AbilityEnding) override;
};
