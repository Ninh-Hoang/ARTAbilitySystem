#include "ARTCharacter/AttributeSet/ARTAttributeSet_Defense.h"

#include "Net/UnrealNetwork.h"

UARTAttributeSet_Defense::UARTAttributeSet_Defense()
{
}

void UARTAttributeSet_Defense::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Defense, Armor, COND_None, REPNOTIFY_Always);
}
