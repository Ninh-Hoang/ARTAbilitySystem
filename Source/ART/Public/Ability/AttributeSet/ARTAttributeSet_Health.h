// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Health.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Health : public UARTAttributeSetBase
{
	GENERATED_BODY()
	
public:
	UARTAttributeSet_Health();
	
	//Health is important to check if alive or not, there it is in base class
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Health, Health)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Health, Health, OldHealth);
	}

	// MaxHealth is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Health, MaxHealth)

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Health, MaxHealth, OldMaxHealth);
	}

	// HealthRegen is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Health, HealthRegen)

	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Health, HealthRegen, OldHealthRegen);
	}

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
