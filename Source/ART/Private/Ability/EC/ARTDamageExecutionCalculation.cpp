// Fill out your copyright notice in the Description page of Project Settings.

/*
 * TODO: This is Project specific, remove from Plugin
 */

#include "Ability/EC/ARTDamageExecutionCalculation.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Damage.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Defense.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Offense.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Shield.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct ARTDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	DECLARE_ATTRIBUTE_CAPTUREDEF(Shield);

	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	ARTDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// here could be like AttackPower attributes that you might want.

		// Capture the Source's attack. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTAttributeSet_Offense, AttackPower, Source, false);

		// Capture the Target's Armor. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTAttributeSet_Defense, Armor, Target, false);

		// Capture the Target's Shield. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTAttributeSet_Shield, Shield, Target, false);
		
		// The Target's received Damage. This is the value of health that will be subtracted on the Target. We're not capturing this.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTAttributeSet_Damage, Damage, Target, false);
	}
};

static const ARTDamageStatics& DamageStatics()
{
	static ARTDamageStatics DStatics;
	return DStatics;
}

UARTDamageExecutionCalculation::UARTDamageExecutionCalculation()
{
	CritMultiplier = 1.5f;

	ValidTransientAggregatorIdentifiers.AddTag(FGameplayTag::RequestGameplayTag("Data.Damage"));
	
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ShieldDef);
}

void UARTDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	float AttackPower = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters,
	                                                           AttackPower);
	AttackPower = FMath::Max(AttackPower, 0.0f);

	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max(Armor, 0.0f);
	
	float Shield = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ShieldDef, EvaluationParameters, Shield);
	Shield = FMath::Max(Shield, 0.0f);
	
	// get temporal data
	float Damage = 0.f;
	ExecutionParams.AttemptCalculateTransientAggregatorMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), EvaluationParameters, Damage);
	Damage = FMath::Max(Damage, 0.f);

	const float UnmitigatedDamage = Damage * AttackPower; // Can multiply any damage boosters here

	//formular: only health is under armor mitigation from damage
	float MitigatedDamage = UnmitigatedDamage;
	
	//if Damage exceed shield, calculate damage to health with armor modification
	if (MitigatedDamage > Shield)
	{
		MitigatedDamage = Shield + (MitigatedDamage - Shield) * (100 / (Armor + 100));
	}

	// Set the Target's damage meta attribute
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	
}
