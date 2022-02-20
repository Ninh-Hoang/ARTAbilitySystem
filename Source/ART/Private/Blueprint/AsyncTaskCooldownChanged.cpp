// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/AsyncTaskCooldownChanged.h"

UAsyncTaskCooldownChanged* UAsyncTaskCooldownChanged::ListenForCooldownChange(
	UARTAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer InCooldownTags, bool InUseServerCooldown)
{
	UAsyncTaskCooldownChanged* ListenForCooldownChange = NewObject<UAsyncTaskCooldownChanged>();
	ListenForCooldownChange->ASC = AbilitySystemComponent;
	ListenForCooldownChange->CooldownTags = InCooldownTags;
	ListenForCooldownChange->UseServerCooldown = InUseServerCooldown;

	if (!IsValid(AbilitySystemComponent) || InCooldownTags.Num() < 1)
	{
		ListenForCooldownChange->EndTask();
		return nullptr;
	}

	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		ListenForCooldownChange, &UAsyncTaskCooldownChanged::OnActiveGameplayEffectAddedCallback);

	TArray<FGameplayTag> CooldownTagArray;
	InCooldownTags.GetGameplayTagArray(CooldownTagArray);

	for (FGameplayTag CooldownTag : CooldownTagArray)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::AnyCountChange).AddUObject(
			ListenForCooldownChange, &UAsyncTaskCooldownChanged::CooldownTagChanged);
	}

	return ListenForCooldownChange;
}

void UAsyncTaskCooldownChanged::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);

		TArray<FGameplayTag> CooldownTagArray;
		CooldownTags.GetGameplayTagArray(CooldownTagArray);

		for (FGameplayTag CooldownTag : CooldownTagArray)
		{
			ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::AnyCountChange).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UAsyncTaskCooldownChanged::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target,
                                                                    const FGameplayEffectSpec& SpecApplied,
                                                                    FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	TArray<FGameplayTag> CooldownTagArray;
	CooldownTags.GetGameplayTagArray(CooldownTagArray);

	for (FGameplayTag CooldownTag : CooldownTagArray)
	{
		if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
		{
			float TimeRemaining = 0.0f;
			float Duration = 0.0f;
			int32 Charge = 0;
			// Expecting cooldown tag to always be first tag
			FGameplayTagContainer CooldownTagContainer(GrantedTags.GetByIndex(0));
			GetCooldownRemainingForTag(CooldownTagContainer, TimeRemaining, Duration, Charge);

			if (ASC->GetOwnerRole() == ROLE_Authority)
			{
				// Player is Server
				OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration, Charge);
			}
			else if (!UseServerCooldown && SpecApplied.GetContext().GetAbilityInstance_NotReplicated())
			{
				// Client using predicted cooldown
				OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration, Charge);
			}
			else if (UseServerCooldown && SpecApplied.GetContext().GetAbilityInstance_NotReplicated() == nullptr)
			{
				// Client using Server's cooldown. This is Server's corrective cooldown GE.
				OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, Duration, Charge);
			}
			else if (UseServerCooldown && SpecApplied.GetContext().GetAbilityInstance_NotReplicated())
			{
				// Client using Server's cooldown but this is predicted cooldown GE.
				// This can be useful to gray out abilities until Server's cooldown comes in.
				OnCooldownBegin.Broadcast(CooldownTag, -1.0f, -1.0f, -1);
			}
		}
	}
}

void UAsyncTaskCooldownChanged::CooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		OnCooldownEnd.Broadcast(CooldownTag, -1.0f, -1.0f, -1);
	}
	/*else {
		FGameplayTagContainer CooldownTagContainer;
		CooldownTagContainer.AddTag(CooldownTag);
		float TimeRemaining = 0.f;
		float CooldownDuration = 0.f;
		int32 Charge = 0;
		GetCooldownRemainingForTag(CooldownTagContainer, TimeRemaining, CooldownDuration, Charge);
		OnCooldownBegin.Broadcast(CooldownTag, TimeRemaining, CooldownDuration, Charge);
	}*/
}

bool UAsyncTaskCooldownChanged::GetCooldownRemainingForTag(FGameplayTagContainer InCooldownTags, float& TimeRemaining,
                                                           float& CooldownDuration, int32& Charge)
{
	if (IsValid(ASC) && InCooldownTags.Num() > 0)
	{
		TimeRemaining = 0.f;
		CooldownDuration = 0.f;
		Charge = 0;
		//Charge = ASC->FindAbilityChargeViaCooldownTag(InCooldownTags);

		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(InCooldownTags);
		TArray<TPair<float, float>> DurationAndTimeRemaining = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
		if (DurationAndTimeRemaining.Num() > 0)
		{
			int32 BestIdx = 0;
			float LongestTime = DurationAndTimeRemaining[0].Key;
			for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
			{
				if (DurationAndTimeRemaining[Idx].Key > LongestTime)
				{
					LongestTime = DurationAndTimeRemaining[Idx].Key;
					BestIdx = Idx;
				}
			}

			TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

			return true;
		}
	}

	return false;
}
