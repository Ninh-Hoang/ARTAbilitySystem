#include "Ability/AttributeSet/ARTAttributeSet_Shield.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UARTAttributeSet_Shield::UARTAttributeSet_Shield()
{
}

void UARTAttributeSet_Shield::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}

void UARTAttributeSet_Shield::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}

void UARTAttributeSet_Shield::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if (Attribute == GetMaxShieldAttribute())
	{
		AdjustAttributeForMaxChange(OldValue, NewValue, GetShieldAttribute());
	}
}

void UARTAttributeSet_Shield::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Shield, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Shield, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Shield, ShieldRegen, COND_None, REPNOTIFY_Always);
}
