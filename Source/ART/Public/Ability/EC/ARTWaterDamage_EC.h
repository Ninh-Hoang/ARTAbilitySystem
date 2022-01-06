// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ARTWaterDamage_EC.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTWaterDamage_EC : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UARTWaterDamage_EC();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                                    OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

protected:
	float CritMultiplier;
};
