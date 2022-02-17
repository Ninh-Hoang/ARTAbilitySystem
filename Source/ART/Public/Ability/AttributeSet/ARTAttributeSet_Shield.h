// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Shield.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Shield : public UARTAttributeSetBase
{
	GENERATED_BODY()
public:
	UARTAttributeSet_Shield();
	
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Shield, Shield)

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Shield, Shield, OldShield);
	}

	//max shield
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Shield, MaxShield)

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Shield, MaxShield, OldMaxShield);
	}

	//ShieldRegen
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_ShieldRegen)
	FGameplayAttributeData ShieldRegen;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Shield, ShieldRegen)

	UFUNCTION()
	void OnRep_ShieldRegen(const FGameplayAttributeData& OldShieldRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Shield, ShieldRegen, OldShieldRegen);
	}

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};