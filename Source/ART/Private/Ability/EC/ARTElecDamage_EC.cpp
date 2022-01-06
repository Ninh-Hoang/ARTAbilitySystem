// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/EC/ARTElecDamage_EC.h"
#include "ARTCharacter/ARTCharacterAttributeSet.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct ARTElecDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	DECLARE_ATTRIBUTE_CAPTUREDEF(ElecBonus);

	DECLARE_ATTRIBUTE_CAPTUREDEF(ElecRes);

	DECLARE_ATTRIBUTE_CAPTUREDEF(Shield);

	// Meta attribute that we're passing into the ExecCalc via SetByCaller on the GESpec so we don't capture it.
	// We still need to declare and define it so that we can output to it.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	ARTElecDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// here could be like AttackPower attributes that you might want.

		// Capture the Source's Attack. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, AttackPower, Source, false);

		// Capture the Source's ElecBonus. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, ElecBonus, Source, false);

		// Capture the Target's ElecRes. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, ElecRes, Target, false);

		// Capture the Target's Shield. Don't snapshot (the false).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, Shield, Target, false);

		// The Target's received Damage. This is the value of health that will be subtracted on the Target. We're not capturing this.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UARTCharacterAttributeSet, Damage, Target, false);
	}
};

static const ARTElecDamageStatics& ELecDamageStatics()
{
	static ARTElecDamageStatics DStatics;
	return DStatics;
}

UARTElecDamage_EC::UARTElecDamage_EC()
{
	RelevantAttributesToCapture.Add(ELecDamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(ELecDamageStatics().ElecBonusDef);
	RelevantAttributesToCapture.Add(ELecDamageStatics().ElecResDef);
	RelevantAttributesToCapture.Add(ELecDamageStatics().ShieldDef);
}

void UARTElecDamage_EC::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                               OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor()  : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor()  : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ELecDamageStatics().AttackPowerDef, EvaluationParameters,
	                                                           AttackPower);
	AttackPower = FMath::Max<float>(AttackPower, 0.0f);

	float ElecBonus = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ELecDamageStatics().ElecBonusDef, EvaluationParameters,
	                                                           ElecBonus);

	float ElecRes = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ELecDamageStatics().ElecResDef, EvaluationParameters,
	                                                           ElecRes);

	float Shield = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(ELecDamageStatics().ShieldDef, EvaluationParameters,
	                                                           Shield);
	Shield = FMath::Max<float>(Shield, 0.0f);

	// SetByCaller Damage
	float Damage = FMath::Max<float>(
		Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage.Elec")), false, -1.0f), 0.0f);

	float BaseDamage = Damage * AttackPower;

	float UnmitigatedDamage = BaseDamage * (ElecBonus + 1); // Can multiply any damage boosters here

	//formular: only health is under armor mitigation from damage
	float MitigatedDamage = UnmitigatedDamage;

	//if Damage exceed shield, calculate damage to health with armor modification
	if (MitigatedDamage > Shield)
	{
		MitigatedDamage = Shield + (MitigatedDamage - Shield) * (1 - ElecRes);
	}

	if (MitigatedDamage > 0.f)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(ELecDamageStatics().DamageProperty, EGameplayModOp::Additive,
			                               MitigatedDamage));

		//send event to target that they just took electro damage
		FGameplayEventData EventData;
		EventData.Instigator = SourceActor;
		EventData.Target = TargetActor;
		EventData.EventMagnitude = BaseDamage;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor,
		                                                         FGameplayTag::RequestGameplayTag(
			                                                         FName("Data.Damage.Elec"), false), EventData);
	}
}
