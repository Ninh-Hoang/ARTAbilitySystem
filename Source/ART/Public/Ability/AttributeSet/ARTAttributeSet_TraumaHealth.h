// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/AttributeSet/ARTAttributeSet_Health.h"
#include "ARTAttributeSet_TraumaHealth.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_TraumaHealth : public UARTAttributeSet_Health
{
	GENERATED_BODY()

public:
	UARTAttributeSet_TraumaHealth();
	
	//Health is important to check if alive or not, there it is in base class
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_TraumaHealth)
	FGameplayAttributeData TraumaHealth;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_TraumaHealth, TraumaHealth)

	UFUNCTION()
	void OnRep_TraumaHealth(const FGameplayAttributeData& OldTraumaHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_TraumaHealth, TraumaHealth, OldTraumaHealth);
	}

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
