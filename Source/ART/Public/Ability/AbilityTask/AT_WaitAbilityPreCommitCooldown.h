// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitAbilityPreCommitCooldown.generated.h"

class UARTGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWaitAbilityPreCommitCooldownDelegate, UARTGameplayAbility*, ActivatedAbility, const FGameplayEffectSpecHandle&, CooldownEffectHandle);
/**
 * 
 */
UCLASS()
class ART_API UAT_WaitAbilityPreCommitCooldown : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE", DisplayName = "Wait Ability Pre Cooldown Commit"))
	static UAT_WaitAbilityPreCommitCooldown* WaitFAbilityPreCooldownCommit(UGameplayAbility* OwningAbility, FGameplayTagContainer WithTags, FGameplayTagContainer WithoutTags, bool TriggerOnce=true);

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE", DisplayName = "Wait Ability Pre Cooldown Commit Query"))
	static UAT_WaitAbilityPreCommitCooldown* WaitFAbilityPreCooldownCommit_Query(UGameplayAbility* OwningAbility, FGameplayTagQuery Query, bool TriggerOnce=true);

	UPROPERTY(BlueprintAssignable)
	FWaitAbilityPreCommitCooldownDelegate PreCooldownCommit;
	
	virtual void Activate() override;

	UFUNCTION()
	void OnPreAbilityCooldownCommit(UARTGameplayAbility* CommitingAbility, FGameplayEffectSpecHandle& CooldownEffectSpecHandle);

	FGameplayTagContainer WithTags;
	FGameplayTagContainer WithoutTags;
	bool TriggerOnce;

	FGameplayTagQuery Query;

protected:

	virtual void OnDestroy(bool AbilityEnded) override;

	FDelegateHandle OnAbilityPreCommitCooldown;
};
