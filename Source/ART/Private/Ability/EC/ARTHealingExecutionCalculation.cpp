// Fill out your copyright notice in the Description page of Project Settings.

/*
 *TODO: This is Project specific, remove from Plugin
 */

#include "Ability/EC/ARTHealingExecutionCalculation.h"
#include "Ability/ARTAbilitySystemComponent.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct ARTHealingStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Healing);

	ARTHealingStatics()
	{
		// Snapshot happens at time of GESpec creation

		// here could be like HealingPower attributes that you might want.

		//capture Target's Health. Don't snapshot (false arg).
		//DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, Health, Target, false);

		//the target's receive Healing. This is the value of health that will apply on target. We are not capturing this
		//DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, Healing, Target, false);
	}
};

static const ARTHealingStatics& HealingStatics()
{
	static ARTHealingStatics HStatics;
	return HStatics;
}

UARTHealingExecutionCalculation::UARTHealingExecutionCalculation()
{
	CritMultiplier = 1.5;

	RelevantAttributesToCapture.Add(HealingStatics().HealthDef);
}

void UARTHealingExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor()  : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor()  : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	//gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Health = 0.0f;
	ExecutionParams.
		AttemptCalculateCapturedAttributeMagnitude(HealingStatics().HealthDef, EvaluationParameters, Health);
	Health = FMath::Max<float>(Health, 0.0f);

	//SetByCaller Healing, healing amount is -1 if data not found, thus negate later calculation due condition > 0
	float Healing = FMath::Max<float>(
		Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Healing")), false, -1.0f), 0.0f);

	float UnModifiedHealing = Healing; //can apply healing booster here

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
