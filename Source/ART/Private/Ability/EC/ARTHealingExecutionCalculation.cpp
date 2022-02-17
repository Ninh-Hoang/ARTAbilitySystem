// Fill out your copyright notice in the Description page of Project Settings.

/*
 *TODO: This is Project specific, remove from Plugin
 */

#include "Ability/EC/ARTHealingExecutionCalculation.h"

#include "Ability/ARTGlobalTags.h"
#include "Ability/AttributeSet/ARTAttributeSet_Healing.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct ARTHealingStatics
{
	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Healing);

	ARTHealingStatics()
	{
		// Snapshot happens at time of GESpec creation

		// here could be like HealingPower attributes that you might want.
		
		//the target's receive Healing. This is the value of health that will apply on target. We are not capturing this
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTAttributeSet_Healing, Healing, Target, false);
	}
};

static const ARTHealingStatics& HealingStatics()
{
	static ARTHealingStatics HStatics;
	return HStatics;
}

UARTHealingExecutionCalculation::UARTHealingExecutionCalculation()
{
	ValidTransientAggregatorIdentifiers.AddTag(FGameplayTag::RequestGameplayTag("Data.Healing"));
	
	RelevantAttributesToCapture.Add(HealingStatics().HealingDef);
}

void UARTHealingExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	//gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	//SetByCaller Healing, healing amount is -1 if data not found, thus negate later calculation due condition > 0
	float Healing = 0.f;
	ExecutionParams.AttemptCalculateTransientAggregatorMagnitude(FARTGlobalTags::Get().Data_Healing, EvaluationParameters,
															   Healing);
	Healing = FMath::Max(Healing, 0.0f);

	const float UnModifiedHealing = Healing; //can apply healing booster here

	//Check for crit, incase need in the future

	float ModifiedHealing = UnModifiedHealing;

	if (ModifiedHealing > 0.f)
	{
		//Set the Target's healing meta attribute
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(HealingStatics().HealingProperty, EGameplayModOp::Additive,
			                               ModifiedHealing));
	}
}
