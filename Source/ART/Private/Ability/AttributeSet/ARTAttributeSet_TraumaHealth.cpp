// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/ARTAttributeSet_TraumaHealth.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UARTAttributeSet_TraumaHealth::UARTAttributeSet_TraumaHealth()
{
}

void UARTAttributeSet_TraumaHealth::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		//update health when trauma health change
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth() - GetTraumaHealth());
	}
	if (Attribute == GetTraumaHealthAttribute())
	{
		//update health when trauma health change
		NewValue = FMath::Clamp(NewValue, 0.0f, MaxHealth.GetBaseValue());
	}
}

void UARTAttributeSet_TraumaHealth::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		//update health when trauma health change
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth() - GetTraumaHealth());
	}
	if (Attribute == GetTraumaHealthAttribute())
	{
		//update health when trauma health change
		NewValue = FMath::Clamp(NewValue, 0.0f, MaxHealth.GetBaseValue());
	}
}

void UARTAttributeSet_TraumaHealth::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	//super of this change deal with MaxHealth
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if(Attribute == GetTraumaHealthAttribute())
	{
		//recalculate health
		GetOwningAbilitySystemComponent()->ApplyModToAttributeUnsafe(GetHealthAttribute(), EGameplayModOp::Additive, 0.f);
	}
}

void UARTAttributeSet_TraumaHealth::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_TraumaHealth, TraumaHealth, COND_None, REPNOTIFY_Always);
}
