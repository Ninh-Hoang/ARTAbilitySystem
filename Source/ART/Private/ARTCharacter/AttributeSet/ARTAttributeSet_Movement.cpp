#include "ARTCharacter/AttributeSet/ARTAttributeSet_Movement.h"

#include "Ability/FARTAggregatorEvaluateMetaDataLibrary.h"
#include "Net/UnrealNetwork.h"

UARTAttributeSet_Movement::UARTAttributeSet_Movement()
{
}

void UARTAttributeSet_Movement::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetMoveSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 1000);
	}
	else if (Attribute == GetRotateRateAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 2000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 2000);
	}
}

void UARTAttributeSet_Movement::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetMoveSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 1000);
	}
	else if (Attribute == GetRotateRateAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 2000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 2000);
	}
}

void UARTAttributeSet_Movement::OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute,
	FAggregator* NewAggregator) const
{
	Super::OnAttributeAggregatorCreated(Attribute, NewAggregator);

	if (!NewAggregator)
	{
		return;
	}

	if (Attribute == GetMoveSpeedAttribute())
	{
		NewAggregator->EvaluationMetaData = &
			FARTAggregatorEvaluateMetaDataLibrary::MostNegativeMod_MostPositiveModPerClass;
	}
}

void UARTAttributeSet_Movement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Movement, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTAttributeSet_Movement, RotateRate, COND_None, REPNOTIFY_Always);
}


