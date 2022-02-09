#include "Ability/FARTAggregatorEvaluateMetaDataLibrary.h"
#include "AbilitySystemComponent.h"

/** Custom functions. The idea here is that we may want to mix and match these (if FAggregatorEvaluateMetaData starts to hold more than just the qualifier functions) */
void QualifierFunc_MostNegativeMod_AllPositiveMods(const FAggregatorEvaluateParameters& EvalParameters,
                                                   const FAggregator* Aggregator)
{
	// We want to inhibit all qualified negative effects except for the most severe. We want to leave positive modifiers alone
	const FAggregatorMod* MostNegativeMod = nullptr;
	float CurrentMostNegativeDelta = 0.f;
	float BaseValue = Aggregator->GetBaseValue();

	Aggregator->ForEachMod([&](const FAggregatorModInfo& ModInfo)
	{
		if (!ModInfo.Mod->Qualifies())
		{
			// Mod doesn't qualify (for other reasons) so ignore
			return;
		}

		float ExpectedDelta = 0.f;
		switch (ModInfo.Op)
		{
		case EGameplayModOp::Additive:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude;
			break;
		case EGameplayModOp::Multiplicitive:
			ExpectedDelta = (BaseValue * ModInfo.Mod->EvaluatedMagnitude) - BaseValue;
			break;
		case EGameplayModOp::Division:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude > 0.f
				                ? ((BaseValue / ModInfo.Mod->EvaluatedMagnitude) - BaseValue)
				                : 0.f;
			break;
		case EGameplayModOp::Override:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude - BaseValue;
			break;
		}

		// If its a negative mod
		if (ExpectedDelta < 0.f)
		{
			// Turn it off no matter what (we will only enable to most negative at the end)
			ModInfo.Mod->SetExplicitQualifies(false);

			// If its the most negative, safe a pointer to it so we can enable it once we finish
			if (ExpectedDelta < CurrentMostNegativeDelta)
			{
				CurrentMostNegativeDelta = ExpectedDelta;
				MostNegativeMod = ModInfo.Mod;
			}
		}
	});

	if (MostNegativeMod)
	{
		MostNegativeMod->SetExplicitQualifies(true);
	}
}

/** static FAggregatorEvaluateMetaDatas that use the above functions */
FAggregatorEvaluateMetaData FARTAggregatorEvaluateMetaDataLibrary::MostNegativeMod_AllPositiveMods(
	QualifierFunc_MostNegativeMod_AllPositiveMods);

void QualifierFunc_MostNegativeMod_MostPositiveModPerClass(const FAggregatorEvaluateParameters EvalParameter,
                                                           const FAggregator* Aggregator)
{
	//filter out all weaker negative mod and leave every positive mod active
	QualifierFunc_MostNegativeMod_AllPositiveMods(EvalParameter, Aggregator);

	//inhibit all negative effect except the most severe, leave negative modifiers alone
	TMap<UClass*, const FAggregatorMod*> MostPositiveModForClass;
	TMap<UClass*, float> MostPositiveDeltaPerClass;

	float BaseValue = Aggregator->GetBaseValue();

	Aggregator->ForEachMod([&](const FAggregatorModInfo& ModInfo)
	{
		if (!ModInfo.Mod->Qualifies())
			return;

		FActiveGameplayEffectHandle Handle = ModInfo.Mod->ActiveHandle;

		UClass* EffectClass = Handle.GetOwningAbilitySystemComponent()->GetActiveGameplayEffect(Handle)->Spec.Def->
		                             GetClass();

		float ExpectedDelta = 0.0f;

		switch (ModInfo.Op)
		{
		case EGameplayModOp::Additive:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude;
			break;
		case EGameplayModOp::Multiplicitive:
			ExpectedDelta = (BaseValue * ModInfo.Mod->EvaluatedMagnitude) - BaseValue;
			break;
		case EGameplayModOp::Division:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude > 0.f
				                ? ((BaseValue / ModInfo.Mod->EvaluatedMagnitude) - BaseValue)
				                : 0.f;
			break;
		case EGameplayModOp::Override:
			ExpectedDelta = ModInfo.Mod->EvaluatedMagnitude - BaseValue;
			break;
		}

		//turn it off, enable the most positive one later
		ModInfo.Mod->SetExplicitQualifies(false);

		//if it is the first one of its class, add it to collection
		if (!MostPositiveDeltaPerClass.Contains(EffectClass))
		{
			MostPositiveDeltaPerClass.Add(EffectClass, ExpectedDelta);
			MostPositiveModForClass.Add(EffectClass, ModInfo.Mod);
		}

		// If its the most positive, safe a pointer to it so we can enable it once we finish
		if (MostPositiveDeltaPerClass[EffectClass] < ExpectedDelta)
		{
			MostPositiveDeltaPerClass[EffectClass] = ExpectedDelta;
			MostPositiveModForClass[EffectClass] = ModInfo.Mod;
		}

		//enable the most positive mod for each class
		TArray<UClass*> RegisteredClasses;
		MostPositiveDeltaPerClass.GetKeys(RegisteredClasses);

		for (UClass* Class : RegisteredClasses)
		{
			MostPositiveModForClass[Class]->SetExplicitQualifies(true);
		}
	});
}

/** static FAggregatorEvaluateMetaDatas that use the above functions */
FAggregatorEvaluateMetaData FARTAggregatorEvaluateMetaDataLibrary::MostNegativeMod_MostPositiveModPerClass(
	QualifierFunc_MostNegativeMod_MostPositiveModPerClass);
