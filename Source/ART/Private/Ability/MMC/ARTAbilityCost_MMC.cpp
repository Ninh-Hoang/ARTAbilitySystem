// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/MMC/ARTAbilityCost_MMC.h"
#include <ARTCharacter/ARTCharacterAttributeSet.h>
#include "Ability/ARTGameplayAbility.h"


UARTAbilityCost_MMC::UARTAbilityCost_MMC()
{
}

float UARTAbilityCost_MMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const UARTGameplayAbility* Ability = Cast<UARTGameplayAbility
	>(Spec.GetContext().GetAbilityInstance_NotReplicated());

	if (!Ability)
	{
		return 0.0f;
	}

	return Ability->Cost.GetValueAtLevel(Ability->GetAbilityLevel());
}
