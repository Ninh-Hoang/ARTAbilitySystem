// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/ARTAttributeSetBase.h"
#include "AbilitySystemComponent.h"

UARTAttributeSetBase::UARTAttributeSetBase()
{
}

void UARTAttributeSetBase::AdjustAttributeForMaxChange(float OldMaxValue, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();

	if (!FMath::IsNearlyEqual(OldMaxValue, NewMaxValue) && AbilityComp)
	{
			AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, 0.f);
	}
}

