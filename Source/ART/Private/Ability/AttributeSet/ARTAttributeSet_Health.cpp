// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/ARTAttributeSet_Health.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UARTAttributeSet_Health::UARTAttributeSet_Health()
{
}

void UARTAttributeSet_Health::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UARTAttributeSet_Health::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UARTAttributeSet_Health::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(OldValue, NewValue, GetHealthAttribute());
	}
}

void UARTAttributeSet_Health::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Health, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Health, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Health, HealthRegen, COND_None, REPNOTIFY_Always);
}
