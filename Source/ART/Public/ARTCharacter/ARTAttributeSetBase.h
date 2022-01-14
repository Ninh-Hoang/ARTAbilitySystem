// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARTAttributeSetBase.generated.h"

/**
 * 
 */
// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ART_API UARTAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UARTAttributeSetBase();

	//Health is important to check if alive or not, there it is in base class
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, Health)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSetBase, Health, OldHealth);
	}

	// MaxHealth is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, MaxHealth)

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSetBase, MaxHealth, OldMaxHealth);
	}

	//MoveSpeed
	UPROPERTY(BlueprintReadOnly, Category = "MoveSpeed", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, MoveSpeed)

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSetBase, MoveSpeed, OldMoveSpeed);
	}

	//RotateRate
	UPROPERTY(BlueprintReadOnly, Category = "RotateRate", ReplicatedUsing = OnRep_RotateRate)
	FGameplayAttributeData RotateRate;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, RotateRate)

	UFUNCTION()
	void OnRep_RotateRate(const FGameplayAttributeData& OldRotateRate)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSetBase, RotateRate, OldRotateRate);
	}

	// Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Damage", meta = (HideFromLevelInfos))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, Damage)

	// Healing is a meta attribute used by the HealingExecution to calculate final healing, which then turns into +Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Healing", meta = (HideFromLevelInfos))
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UARTAttributeSetBase, Healing)
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
                                     const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
                                     const FGameplayAttribute& AffectedAttributeProperty);
};
