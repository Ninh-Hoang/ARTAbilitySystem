// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTAbilitySystemGlobals.h"

#include "Ability/ARTAbilitySystemComponent.h"
#include "Ability/ARTGameplayEffectTypes.h"
#include "Ability/ARTGameplayAbilityTypes.h"

UARTAbilitySystemGlobals::UARTAbilitySystemGlobals()
{
}

void UARTAbilitySystemGlobals::InitGlobalTags()
{
	Super::InitGlobalTags();

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	//KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}

FGameplayEffectContext* UARTAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FARTGameplayEffectContext();
}

FGameplayAbilityActorInfo* UARTAbilitySystemGlobals::AllocAbilityActorInfo() const
{
	return new FARTGameplayAbilityActorInfo();
}
