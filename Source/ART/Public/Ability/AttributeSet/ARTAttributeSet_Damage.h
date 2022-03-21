// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Damage.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Damage : public UARTAttributeSetBase
{
	GENERATED_BODY()

public:
	UARTAttributeSet_Damage();
	
	// Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Damage", meta = (HideFromLevelInfos))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Damage, Damage)
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void FinalDamageDealing(float LocalDamage, const FHitResult* Hit);
};