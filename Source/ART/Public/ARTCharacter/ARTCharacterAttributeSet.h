// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ARTAttributeSetBase.h"

#include "ARTCharacterAttributeSet.generated.h"

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
class ART_API UARTCharacterAttributeSet : public UARTAttributeSetBase
{
	GENERATED_BODY()

public:
	UARTCharacterAttributeSet();

	// AttributeSet Overrides
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute, FAggregator* NewAggregator) const override;

	virtual void FinalDamageDealing(float LocalDamage, const FHitResult* Hit);
	

	//AttackPower
	UPROPERTY(BlueprintReadOnly, Category = "AttackPower", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, AttackPower)

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, AttackPower, OldAttackPower);
	}

	//CritRate
	UPROPERTY(BlueprintReadOnly, Category = "Crit", ReplicatedUsing = OnRep_CritRate)
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, CritRate)

	UFUNCTION()
	void OnRep_CritRate(const FGameplayAttributeData& OldCritRate)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, CritRate, OldCritRate);
	}

	//CritMultiplier
	UPROPERTY(BlueprintReadOnly, Category = "Crit", ReplicatedUsing = OnRep_CritMultiplier)
	FGameplayAttributeData CritMultiplier;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, CritMultiplier)

	UFUNCTION()
	void OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, CritMultiplier, OldCritMultiplier);
	}

	//reaction mastery
	UPROPERTY(BlueprintReadOnly, Category = "ReactionMastery", ReplicatedUsing = OnRep_ReactMas)
	FGameplayAttributeData ReactMas;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ReactMas)

	UFUNCTION()
	void OnRep_ReactMas(const FGameplayAttributeData& OldReactMas)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ReactMas, OldReactMas);
	}

	//physical damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Phys", ReplicatedUsing = OnRep_PhysBonus)
	FGameplayAttributeData PhysBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PhysBonus)

	UFUNCTION()
	void OnRep_PhysBonus(const FGameplayAttributeData& OldPhysBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, PhysBonus, OldPhysBonus);
	}

	//physical damage res
	UPROPERTY(BlueprintReadOnly, Category = "Phys", ReplicatedUsing = OnRep_PhysRes)
	FGameplayAttributeData PhysRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PhysRes)

	UFUNCTION()
	void OnRep_PhysRes(const FGameplayAttributeData& OldPhysRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, PhysRes, OldPhysRes);
	}

	//Armor
	UPROPERTY(BlueprintReadOnly, Category = "Armor", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Armor)

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Armor, OldArmor);
	}

	//healing bonus
	UPROPERTY(BlueprintReadOnly, Category = "Heal", ReplicatedUsing = OnRep_HealBonus)
	FGameplayAttributeData HealBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, HealBonus)

	UFUNCTION()
	void OnRep_HealBonus(const FGameplayAttributeData& OldHealBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, HealBonus, OldHealBonus);
	}

	//incoming healing bonus
	UPROPERTY(BlueprintReadOnly, Category = "Heal", ReplicatedUsing = OnRep_IncomingHealBonus)
	FGameplayAttributeData IncomingHealBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, IncomingHealBonus)

	UFUNCTION()
	void OnRep_IncomingHealBonus(const FGameplayAttributeData& OldIncomingHealBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, IncomingHealBonus, OldIncomingHealBonus);
	}

	//void damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Void", ReplicatedUsing = OnRep_VoidBonus)
	FGameplayAttributeData VoidBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, VoidBonus)

	UFUNCTION()
	void OnRep_VoidBonus(const FGameplayAttributeData& OldVoidBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, VoidBonus, OldVoidBonus);
	}

	//void damage res
	UPROPERTY(BlueprintReadOnly, Category = "Void", ReplicatedUsing = OnRep_VoidRes)
	FGameplayAttributeData VoidRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, VoidRes)

	UFUNCTION()
	void OnRep_VoidRes(const FGameplayAttributeData& OldVoidRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, VoidRes, OldVoidRes);
	}

	//heat damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Heat", ReplicatedUsing = OnRep_HeatBonus)
	FGameplayAttributeData HeatBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, HeatBonus)

	UFUNCTION()
	void OnRep_HeatBonus(const FGameplayAttributeData& OldHeatBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, HeatBonus, OldHeatBonus);
	}

	//heat damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Heat", ReplicatedUsing = OnRep_HeatRes)
	FGameplayAttributeData HeatRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, HeatRes)

	UFUNCTION()
	void OnRep_HeatRes(const FGameplayAttributeData& OldHeatRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, HeatRes, OldHeatRes);
	}

	//cold damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Cold", ReplicatedUsing = OnRep_ColdBonus)
	FGameplayAttributeData ColdBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ColdBonus)

	UFUNCTION()
	void OnRep_ColdBonus(const FGameplayAttributeData& OldColdBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ColdBonus, OldColdBonus);
	}

	//cold damage res
	UPROPERTY(BlueprintReadOnly, Category = "Cold", ReplicatedUsing = OnRep_ColdRes)
	FGameplayAttributeData ColdRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ColdRes)

	UFUNCTION()
	void OnRep_ColdRes(const FGameplayAttributeData& OldColdRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ColdRes, OldColdRes);
	}

	//electricity damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Elec", ReplicatedUsing = OnRep_ElecBonus)
	FGameplayAttributeData ElecBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ElecBonus)

	UFUNCTION()
	void OnRep_ElecBonus(const FGameplayAttributeData& OldElecBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ElecBonus, OldElecBonus);
	}

	//electricity damage res
	UPROPERTY(BlueprintReadOnly, Category = "Elec", ReplicatedUsing = OnRep_ElecRes)
	FGameplayAttributeData ElecRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ElecRes)

	UFUNCTION()
	void OnRep_ElecRes(const FGameplayAttributeData& OldElecRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ElecRes, OldElecRes);
	}

	//water damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Water", ReplicatedUsing = OnRep_WaterBonus)
	FGameplayAttributeData WaterBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, WaterBonus)

	UFUNCTION()
	void OnRep_WaterBonus(const FGameplayAttributeData& OldWaterBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, WaterBonus, OldWaterBonus);
	}

	//water damage res
	UPROPERTY(BlueprintReadOnly, Category = "Water", ReplicatedUsing = OnRep_WaterRes)
	FGameplayAttributeData WaterRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, WaterRes)

	UFUNCTION()
	void OnRep_WaterRes(const FGameplayAttributeData& OldWaterRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, WaterRes, OldWaterRes);
	}

	//earth damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Earth", ReplicatedUsing = OnRep_EarthBonus)
	FGameplayAttributeData EarthBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, EarthBonus)

	UFUNCTION()
	void OnRep_EarthBonus(const FGameplayAttributeData& OldEarthBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, EarthBonus, OldEarthBonus);
	}

	//earth damage res
	UPROPERTY(BlueprintReadOnly, Category = "Earth", ReplicatedUsing = OnRep_EarthRes)
	FGameplayAttributeData EarthRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, EarthRes)

	UFUNCTION()
	void OnRep_EarthRes(const FGameplayAttributeData& OldEarthRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, EarthRes, OldEarthRes);
	}

	//air damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Air", ReplicatedUsing = OnRep_AirBonus)
	FGameplayAttributeData AirBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, AirBonus)

	UFUNCTION()
	void OnRep_AirBonus(const FGameplayAttributeData& OldAirBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, AirBonus, OldAirBonus);
	}

	//air damage res
	UPROPERTY(BlueprintReadOnly, Category = "Air", ReplicatedUsing = OnRep_AirRes)
	FGameplayAttributeData AirRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, AirRes)

	UFUNCTION()
	void OnRep_AirRes(const FGameplayAttributeData& OldAirRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, AirRes, OldAirRes);
	}

	//life damage bonus
	UPROPERTY(BlueprintReadOnly, Category = "Life", ReplicatedUsing = OnRep_LifeBonus)
	FGameplayAttributeData LifeBonus;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, LifeBonus)

	UFUNCTION()
	void OnRep_LifeBonus(const FGameplayAttributeData& OldLifeBonus)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, LifeBonus, OldLifeBonus);
	}

	//life damage res
	UPROPERTY(BlueprintReadOnly, Category = "Life", ReplicatedUsing = OnRep_LifeRes)
	FGameplayAttributeData LifeRes;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, LifeRes)

	UFUNCTION()
	void OnRep_LifeRes(const FGameplayAttributeData& OldLifeRes)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, LifeRes, OldLifeRes);
	}

	//shield
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Shield)

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Shield, OldShield);
	}

	//max shield
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, MaxShield)

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, MaxShield, OldMaxShield);
	}

	//ShieldRegen
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_ShieldRegen)
	FGameplayAttributeData ShieldRegen;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, ShieldRegen)

	UFUNCTION()
	void OnRep_ShieldRegen(const FGameplayAttributeData& OldShieldRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, ShieldRegen, OldShieldRegen);
	}

	//Health
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Health)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldHealth);
	}

	// MaxHealth is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, MaxHealth)

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, MaxHealth, OldMaxHealth);
	}

	// HealthRegen is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthRegen)
	FGameplayAttributeData HealthRegen;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, HealthRegen)

	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, HealthRegen, OldHealthRegen);
	}

	//PartHealthA
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthA", ReplicatedUsing = OnRep_PartHealthA)
	FGameplayAttributeData PartHealthA;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthA)

    UFUNCTION()
    void OnRep_PartHealthA(const FGameplayAttributeData& OldPartHealthA)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthA);
	}

	//PartHealthB
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthB", ReplicatedUsing = OnRep_PartHealthB)
	FGameplayAttributeData PartHealthB;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthB)

    UFUNCTION()
    void OnRep_PartHealthB(const FGameplayAttributeData& OldPartHealthB)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthB);
	}

	//PartHealthC
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthC", ReplicatedUsing = OnRep_PartHealthC)
	FGameplayAttributeData PartHealthC;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthC)

    UFUNCTION()
    void OnRep_PartHealthC(const FGameplayAttributeData& OldPartHealthC)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthC);
	}

	//PartHealthD
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthD", ReplicatedUsing = OnRep_PartHealthD)
	FGameplayAttributeData PartHealthD;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthD)

    UFUNCTION()
    void OnRep_PartHealthD(const FGameplayAttributeData& OldPartHealthD)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthD);
	}

	//PartHealthE
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthE", ReplicatedUsing = OnRep_PartHealthE)
	FGameplayAttributeData PartHealthE;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthE)

    UFUNCTION()
    void OnRep_PartHealthE(const FGameplayAttributeData& OldPartHealthE)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthE);
	}

	//PartHealthF
	UPROPERTY(BlueprintReadOnly, Category = "PartHealthF", ReplicatedUsing = OnRep_PartHealthF)
	FGameplayAttributeData PartHealthF;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, PartHealthF)

    UFUNCTION()
    void OnRep_PartHealthF(const FGameplayAttributeData& OldPartHealthF)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Health, OldPartHealthF);
	}

	//Energy
	UPROPERTY(BlueprintReadOnly, Category = "Energy", ReplicatedUsing = OnRep_Energy)
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Energy)

	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldEnergy)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Energy, OldEnergy);
	}

	// MaxEnergy is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Energy", ReplicatedUsing = OnRep_MaxEnergy)
	FGameplayAttributeData MaxEnergy;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, MaxEnergy)

	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, MaxEnergy, OldMaxEnergy);
	}

	// EnergyRegen is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Energy", ReplicatedUsing = OnRep_EnergyRegen)
	FGameplayAttributeData EnergyRegen;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, EnergyRegen)

	UFUNCTION()
	void OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, EnergyRegen, OldEnergyRegen);
	}

	//Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Stamina)

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, Stamina, OldStamina);
	}

	// MaxStamina is its own attribute since GameplayEffects may modify it
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, MaxStamina)

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, MaxStamina, OldMaxStamina);
	}

	//Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_StaminaRegen)
	FGameplayAttributeData StaminaRegen;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, StaminaRegen)

	UFUNCTION()
	void OnRep_StaminaRegen(const FGameplayAttributeData& OldStaminaRegen)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, StaminaRegen, OldStaminaRegen);
	}

	//MoveSpeed
	UPROPERTY(BlueprintReadOnly, Category = "MoveSpeed", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, MoveSpeed)

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, MoveSpeed, OldMoveSpeed);
	}

	//RotateRate
	UPROPERTY(BlueprintReadOnly, Category = "RotateRate", ReplicatedUsing = OnRep_RotateRate)
	FGameplayAttributeData RotateRate;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, RotateRate)

	UFUNCTION()
	void OnRep_RotateRate(const FGameplayAttributeData& OldRotateRate)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, RotateRate, OldRotateRate);
	}

	//XP mod
	UPROPERTY(BlueprintReadOnly, Category = "XPMod", ReplicatedUsing = OnRep_XPMod)
	FGameplayAttributeData XPMod;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, XPMod)

	UFUNCTION()
	void OnRep_XPMod(const FGameplayAttributeData& OldXPMod)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, XPMod, OldXPMod);
	}

	//En mod
	UPROPERTY(BlueprintReadOnly, Category = "EnMod", ReplicatedUsing = OnRep_EnMod)
	FGameplayAttributeData EnMod;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, EnMod)

    UFUNCTION()
    void OnRep_EnMod(const FGameplayAttributeData& OldEnMod)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTCharacterAttributeSet, XPMod, OldEnMod);
	}

	// Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Damage", meta = (HideFromLevelInfos))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Damage)

	// Healing is a meta attribute used by the HealingExecution to calculate final healing, which then turns into +Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Healing", meta = (HideFromLevelInfos))
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UARTCharacterAttributeSet, Healing)

protected:
	//cache tags
	FGameplayTag StunTag;
	FGameplayTag PartABrokenStateTag;
	FGameplayTag PartBBrokenStateTag;
	FGameplayTag PartCBrokenStateTag;
	FGameplayTag PartDBrokenStateTag;
	FGameplayTag PartEBrokenStateTag;
	FGameplayTag PartFBrokenStateTag;
	FGameplayTag PartABrokenEventTag;
	FGameplayTag PartBBrokenEventTag;
	FGameplayTag PartCBrokenEventTag;
	FGameplayTag PartDBrokenEventTag;
	FGameplayTag PartEBrokenEventTag;
	FGameplayTag PartFBrokenEventTag;
};
