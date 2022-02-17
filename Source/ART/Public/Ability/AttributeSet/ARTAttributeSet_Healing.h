// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Healing.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Healing : public UARTAttributeSetBase
{
	GENERATED_BODY()

public:
	UARTAttributeSet_Healing();

	// Healing is a meta attribute used by the HealingExecution to calculate final healing, which then turns into +Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Healing", meta = (HideFromLevelInfos))
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Healing, Healing)

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};