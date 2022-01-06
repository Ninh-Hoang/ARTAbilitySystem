// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_WaitGEAppliedSelf_Local.h"

#include "AbilitySystemComponent.h"

UAT_WaitGEAppliedSelf_Local::UAT_WaitGEAppliedSelf_Local(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAT_WaitGEAppliedSelf_Local* UAT_WaitGEAppliedSelf_Local::WaitGameplayEffectAppliedToSelf_Local(UGameplayAbility* OwningAbility, const FGameplayTargetDataFilterHandle InFilter, FGameplayTagRequirements InSourceTagRequirements, FGameplayTagRequirements InTargetTagRequirements, FGameplayTagRequirements InEffectAssetTagRequirements, bool InTriggerOnce, AActor* OptionalExternalOwner, bool InListenForPeriodicEffect)
{
	UAT_WaitGEAppliedSelf_Local* MyObj = NewAbilityTask<UAT_WaitGEAppliedSelf_Local>(OwningAbility);
	MyObj->Filter = InFilter;
	MyObj->SourceTagRequirements = InSourceTagRequirements;
	MyObj->TargetTagRequirements = InTargetTagRequirements;
	MyObj->EffectAssetTagRequirements = InEffectAssetTagRequirements;
	MyObj->TriggerOnce = InTriggerOnce;
	MyObj->SetExternalActor(OptionalExternalOwner);
	MyObj->ListenForPeriodicEffects = InListenForPeriodicEffect;
	return MyObj;
}

UAT_WaitGEAppliedSelf_Local* UAT_WaitGEAppliedSelf_Local::WaitGameplayEffectAppliedToSelf_QueryLocal(UGameplayAbility* OwningAbility, const FGameplayTargetDataFilterHandle InFilter, FGameplayTagQuery InSourceTagQuery, FGameplayTagQuery InTargetTagQuery, FGameplayTagQuery InEffectAssetTagQuery, bool InTriggerOnce, AActor* OptionalExternalOwner, bool InListenForPeriodicEffect)
{
	UAT_WaitGEAppliedSelf_Local* MyObj = NewAbilityTask<UAT_WaitGEAppliedSelf_Local>(OwningAbility);
	MyObj->Filter = InFilter;
	MyObj->SourceTagQuery = InSourceTagQuery;
	MyObj->TargetTagQuery = InTargetTagQuery;
	MyObj->EffectAssetTagQuery = InEffectAssetTagQuery;
	MyObj->TriggerOnce = InTriggerOnce;
	MyObj->SetExternalActor(OptionalExternalOwner);
	MyObj->ListenForPeriodicEffects = InListenForPeriodicEffect;
	return MyObj;
}

void UAT_WaitGEAppliedSelf_Local::OnApplyGameplayEffectCallback_Local(UAbilitySystemComponent* Target,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	bool PassedComparison = false;

	AActor* AvatarActor = Target ? Target->GetAvatarActor_Direct() : nullptr;

	if (!Filter.FilterPassesForActor(AvatarActor))
	{
		return;
	}
	if (!SourceTagRequirements.RequirementsMet(*SpecApplied.CapturedSourceTags.GetAggregatedTags()))
	{
		return;
	}
	if (!TargetTagRequirements.RequirementsMet(*SpecApplied.CapturedTargetTags.GetAggregatedTags()))
	{
		return;
	}
	if (!EffectAssetTagRequirements.RequirementsMet(SpecApplied.Def->InheritableGameplayEffectTags.CombinedTags))
	{
		return;
	}

	if (SourceTagQuery.IsEmpty() == false)
	{
		if (!SourceTagQuery.Matches(*SpecApplied.CapturedSourceTags.GetAggregatedTags()))
		{
			return;
		}
	}

	if (TargetTagQuery.IsEmpty() == false)
	{
		if (!TargetTagQuery.Matches(*SpecApplied.CapturedTargetTags.GetAggregatedTags()))
		{
			return;
		}
	}
	
	if(EffectAssetTagQuery.IsEmpty() == false)
	{
		if(!EffectAssetTagQuery.Matches(SpecApplied.Def->InheritableGameplayEffectTags.CombinedTags))
		{
			return;
		}
	}

	if (Locked)
	{
		ABILITY_LOG(Error, TEXT("WaitGameplayEffectApplied recursion detected. Ability: %s. Applied Spec: %s. This could cause an infinite loop! Ignoring"), *GetNameSafe(Ability), *SpecApplied.ToSimpleString());
		return;
	}
	
	FGameplayEffectSpecHandle	SpecHandle(new FGameplayEffectSpec(SpecApplied));

	{
		TGuardValue<bool> GuardValue(Locked, true);	
		BroadcastDelegate(AvatarActor, SpecHandle, ActiveHandle);
	}

	if (TriggerOnce)
	{
		EndTask();
	}
}

void UAT_WaitGEAppliedSelf_Local::BroadcastDelegate(AActor* Avatar, FGameplayEffectSpecHandle SpecHandle, FActiveGameplayEffectHandle ActiveHandle)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnApplied.Broadcast(Avatar, SpecHandle, ActiveHandle);
	}
}

void UAT_WaitGEAppliedSelf_Local::RegisterDelegate()
{
	OnApplyGameplayEffectCallbackDelegateHandle = GetASC()->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UAT_WaitGEAppliedSelf_Local::OnApplyGameplayEffectCallback_Local);
	if (ListenForPeriodicEffects)
	{
		OnPeriodicGameplayEffectExecuteCallbackDelegateHandle = GetASC()->OnPeriodicGameplayEffectExecuteDelegateOnSelf.AddUObject(this, &UAT_WaitGEAppliedSelf_Local::OnApplyGameplayEffectCallback_Local);
	}
}

void UAT_WaitGEAppliedSelf_Local::RemoveDelegate()
{
	GetASC()->OnGameplayEffectAppliedDelegateToSelf.Remove(OnApplyGameplayEffectCallbackDelegateHandle);
	if (OnPeriodicGameplayEffectExecuteCallbackDelegateHandle.IsValid())
	{
		GetASC()->OnGameplayEffectAppliedDelegateToTarget.Remove(OnPeriodicGameplayEffectExecuteCallbackDelegateHandle);
	}
}