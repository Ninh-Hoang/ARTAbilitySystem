// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_WaitAbilityPreCommitCooldown.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"

UAT_WaitAbilityPreCommitCooldown::UAT_WaitAbilityPreCommitCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAT_WaitAbilityPreCommitCooldown* UAT_WaitAbilityPreCommitCooldown::WaitFAbilityPreCooldownCommit(
	UGameplayAbility* OwningAbility, FGameplayTagContainer InWithTags, FGameplayTagContainer InWithoutTags,
	bool InTriggerOnce)
{
	UAT_WaitAbilityPreCommitCooldown* MyObj = NewAbilityTask<UAT_WaitAbilityPreCommitCooldown>(OwningAbility);
	MyObj->WithTags = InWithTags;
	MyObj->WithoutTags = InWithoutTags;
	MyObj->TriggerOnce = InTriggerOnce;

	return MyObj;
}

UAT_WaitAbilityPreCommitCooldown* UAT_WaitAbilityPreCommitCooldown::WaitFAbilityPreCooldownCommit_Query(
	UGameplayAbility* OwningAbility, FGameplayTagQuery Query, bool InTriggerOnce)
{
	UAT_WaitAbilityPreCommitCooldown* MyObj = NewAbilityTask<UAT_WaitAbilityPreCommitCooldown>(OwningAbility);
	MyObj->Query = Query;
	MyObj->TriggerOnce = InTriggerOnce;
	return MyObj;
}

void UAT_WaitAbilityPreCommitCooldown::Activate()
{
	if (UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(AbilitySystemComponent))	
	{		
		OnAbilityPreCommitCooldown = ASC->PreCommitCooldownEffectCallbacks.AddUObject(this, &UAT_WaitAbilityPreCommitCooldown::OnPreAbilityCooldownCommit);
	}
}

void UAT_WaitAbilityPreCommitCooldown::OnDestroy(bool AbilityEnded)
{
	if (UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(AbilitySystemComponent))	
	{		
		ASC->PreCommitCooldownEffectCallbacks.Remove(OnAbilityPreCommitCooldown);
	}

	Super::OnDestroy(AbilityEnded);
}
void UAT_WaitAbilityPreCommitCooldown::OnPreAbilityCooldownCommit(UARTGameplayAbility* CommitingAbility,
	FGameplayEffectSpecHandle& CooldownEffectSpecHandle)
{
	if(!WithTags.IsEmpty() && !CommitingAbility->AbilityTags.HasAll(WithTags) || !WithoutTags.IsEmpty() && CommitingAbility->AbilityTags.HasAny(WithoutTags))
	{
		return;
	}

	if(!Query.IsEmpty())
	{
		if(!Query.Matches(CommitingAbility->AbilityTags))
		{
			return;
		}
	}
	if(ShouldBroadcastAbilityTaskDelegates())
	{
		PreCooldownCommit.Broadcast(CommitingAbility, CooldownEffectSpecHandle);
	}

	if(TriggerOnce)
	{
		EndTask();
	}
}