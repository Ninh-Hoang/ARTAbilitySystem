// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "ARTInvAbilityTask_SimpleTarget.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UAbilityTask_SimpleInvTarget : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_SimpleInvTarget(const FObjectInitializer& ObjectInitializer);

	virtual FGameplayAbilityTargetDataHandle GenerateTargetHandle();
	virtual void HandleTargetData(const FGameplayAbilityTargetDataHandle& Data);
	virtual void HandleCancelled();


	virtual void Activate() override;
	virtual void OnTargetDataCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);
	virtual void OnTargetDataCancelled();

protected:
	FGameplayAbilityTargetDataHandle ServerTargetData;
	
};
