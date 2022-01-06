// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTGlobalTags.h"
#include "AbilitySystemGlobals.h"

// ---------------------------------------------------------------------------------------------------
// Behaviour tags
// ---------------------------------------------------------------------------------------------------

const FGameplayTag& UARTGlobalTags::Behaviour()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour")));
	return Tag;
}

const FGameplayTag& UARTGlobalTags::Behaviour_Self()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour.Self")));
	return Tag;
}

const FGameplayTag& UARTGlobalTags::Behaviour_Friendly()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour.Friendly")));
	return Tag;
}

const FGameplayTag& UARTGlobalTags::Behaviour_Hostile()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour.Hostile")));
	return Tag;
}

const FGameplayTag& UARTGlobalTags::Behaviour_Neutral()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour.Neutral")));
	return Tag;
}

const FGameplayTag& UARTGlobalTags::Behaviour_Visible()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Behaviour.Visible")));
	return Tag;
}

// ---------------------------------------------------------------------------------------------------
// Ability activation failure tags
// ---------------------------------------------------------------------------------------------------

const FGameplayTag& UARTGlobalTags::AbilityActivationFailure_BlockedTag()
{
	return UAbilitySystemGlobals::Get().ActivateFailTagsBlockedTag;
}

const FGameplayTag& UARTGlobalTags::AbilityActivationFailure_MissingTag()
{
	return UAbilitySystemGlobals::Get().ActivateFailTagsMissingTag;
}

const FGameplayTag& UARTGlobalTags::AbilityActivationFailure_Cooldown()
{
	return UAbilitySystemGlobals::Get().ActivateFailCooldownTag;
}

const FGameplayTag& UARTGlobalTags::AbilityActivationFailure_Cost()
{
	return UAbilitySystemGlobals::Get().ActivateFailCostTag;
}

const FGameplayTag& UARTGlobalTags::AbilityActivationFailure_NoTarget()
{
	static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("AbilityActivationFailure.NoTarget")));
	return Tag;
}