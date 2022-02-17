#include "Ability/AttributeSet/ARTAttributeSet_Offense.h"

#include "Net/UnrealNetwork.h"

UARTAttributeSet_Offense::UARTAttributeSet_Offense()
{
}

void UARTAttributeSet_Offense::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Offense, AttackPower, COND_None, REPNOTIFY_Always);
}
