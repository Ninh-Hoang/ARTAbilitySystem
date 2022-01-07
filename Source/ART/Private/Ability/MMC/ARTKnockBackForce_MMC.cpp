// Fill out your copyright notice in the Description page of Project Settings.

/*
 *TODO: This is Project specific, remove from Plugin
 */

#include "Ability/MMC/ARTKnockBackForce_MMC.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

UARTKnockBackForce_MMC::UARTKnockBackForce_MMC()
{
}

float UARTKnockBackForce_MMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//send event data
	UAbilitySystemComponent* SourceASC = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent();
	AActor* SourceActor = SourceASC->GetAvatarActor();

	return 0.0f;
}