// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTGameplayEffect.h"
#include <GameplayEffectTypes.h>
#include <AbilitySystemComponent.h>

#if WITH_EDITOR
#define GETCURVE_REPORTERROR_WITHPOSTLOAD(Handle) \
	if (Handle.CurveTable) const_cast<UCurveTable*>(Handle.CurveTable)->ConditionalPostLoad(); \
	GETCURVE_REPORTERROR(Handle);

#define GETCURVE_REPORTERROR_WITHPATHNAME_WITHPOSTLOAD(Handle, PathNameString) \
	if (Handle.CurveTable) const_cast<UCurveTable*>(Handle.CurveTable)->ConditionalPostLoad(); \
	GETCURVE_REPORTERROR_WITHPATHNAME(Handle, PathNameString);
#endif // WITH_EDITOR

bool FGameplayEffectEvent::AttemptCalculateMagnitude(const FGameplayEffectSpec& InRelevantSpec,
                                                     OUT float& OutCalculatedMagnitude,
                                                     bool WarnIfSetByCallerFail /*= true*/,
                                                     float DefaultSetbyCaller /*= 0.f*/) const
{
	FString ContextString = FString::Printf(
		TEXT("FGameplayEffectModifierMagnitude::AttemptCalculateMagnitude from effect %s"),
		*InRelevantSpec.ToSimpleString());

	switch (GameplayEventMagnitudeCalculation)
	{
	case EGameplayEffectEventMagnitude::ScalableFloat:
		{
			OutCalculatedMagnitude = ScalableFloatEventMagnitude.GetValueAtLevel(
				InRelevantSpec.GetLevel(), &ContextString);
		}
		break;

	case EGameplayEffectEventMagnitude::SetByCaller:
		{
			if (SetByCallerEventMagnitude.DataTag.IsValid())
			{
				OutCalculatedMagnitude = InRelevantSpec.GetSetByCallerMagnitude(
					SetByCallerEventMagnitude.DataTag, WarnIfSetByCallerFail, DefaultSetbyCaller);
			}
			else
			{
				PRAGMA_DISABLE_DEPRECATION_WARNINGS

				OutCalculatedMagnitude = InRelevantSpec.GetSetByCallerMagnitude(
					SetByCallerEventMagnitude.DataName, WarnIfSetByCallerFail, DefaultSetbyCaller);

				PRAGMA_ENABLE_DEPRECATION_WARNINGS
			}
		}
		break;

	default:
		ABILITY_LOG(Error, TEXT("Unknown MagnitudeCalculationType %d in AttemptCalculateMagnitude"),
		            static_cast<int32>(GameplayEventMagnitudeCalculation));
		OutCalculatedMagnitude = 0.f;
		break;
	}

	return true;
}

bool FGameplayEffectEvent::AttemptReturnGameplayEventTags(const FGameplayTagContainer* InstigatorTags,
                                                          const FGameplayTagContainer* TargetTags,
                                                          OUT FGameplayTag& InEventTag,
                                                          OUT FGameplayTagContainer& InInstigatorTags,
                                                          OUT FGameplayTagContainer& InTargetTags)
{
	InEventTag = GameplayEventTag;
	switch (GameplayEventDirection)
	{
	case EGameplayEffectEventDirection::SourceToTarget:
		{
			UE_LOG(LogTemp, Warning, TEXT("setting"));
			InInstigatorTags = *InstigatorTags;
			InTargetTags = *TargetTags;
		}
		break;
	case EGameplayEffectEventDirection::TargetToSource:
		{
			InInstigatorTags = *TargetTags;
			InTargetTags = *InstigatorTags;
		}
		break;
	default:
		ABILITY_LOG(Error, TEXT("Unknown GameplayEffectEventDirection %d in AttempAssignGameplayEventDataActors"),
		            static_cast<int32>(GameplayEventDirection));
		InInstigatorTags = *InstigatorTags;
		InTargetTags = *TargetTags;
		break;
	}
	return true;
}

bool FGameplayEffectEvent::AttempAssignGameplayEventDataActors(AActor* SourceActor, AActor* TargetActor,
                                                               OUT AActor*& Instigator, OUT AActor*& Target)
{
	switch (GameplayEventDirection)
	{
	case EGameplayEffectEventDirection::SourceToTarget:
		{
			Instigator = SourceActor;
			Target = TargetActor;
		}
		break;
	case EGameplayEffectEventDirection::TargetToSource:
		{
			Instigator = TargetActor;
			Target = SourceActor;
		}
		break;
	default:
		ABILITY_LOG(Error, TEXT("Unknown GameplayEffectEventDirection %d in AttempAssignGameplayEventDataActors"),
		            static_cast<int32>(GameplayEventDirection));
		Instigator = SourceActor;
		Target = TargetActor;
		break;
	}

	return true;
}

bool FGameplayEffectEvent::GetStaticMagnitudeIfPossible(float InLevel, float& OutMagnitude,
                                                        const FString* ContextString /*= nullptr*/) const
{
	if (GameplayEventMagnitudeCalculation == EGameplayEffectEventMagnitude::ScalableFloat)
	{
		OutMagnitude = ScalableFloatEventMagnitude.GetValueAtLevel(InLevel, ContextString);
		return true;
	}

	return false;
}

bool FGameplayEffectEvent::GetSetByCallerDataNameIfPossible(FName& OutDataName) const
{
	if (GameplayEventMagnitudeCalculation == EGameplayEffectEventMagnitude::SetByCaller)
	{
		OutDataName = SetByCallerEventMagnitude.DataName;
		return true;
	}

	return false;
}

bool FGameplayEffectEvent::Serialize(FArchive& Ar)
{
	// Clear properties that are not needed for the chosen calculation type
	if (Ar.IsSaving() && Ar.IsPersistent() && !Ar.IsTransacting())
	{
		if (GameplayEventMagnitudeCalculation != EGameplayEffectEventMagnitude::ScalableFloat)
		{
			ScalableFloatEventMagnitude = FScalableFloat();
		}

		if (GameplayEventMagnitudeCalculation != EGameplayEffectEventMagnitude::SetByCaller)
		{
			SetByCallerEventMagnitude = FSetByCallerFloat();
		}
	}

	// Return false to let normal tagged serialization occur
	return false;
}

#if WITH_EDITOR
FText FGameplayEffectEvent::GetValueForEditorDisplay() const
{
	switch (GameplayEventMagnitudeCalculation)
	{
	case EGameplayEffectEventMagnitude::ScalableFloat:
		return FText::Format(
			NSLOCTEXT("GameplayEffect", "ScalableFloatModifierMagnitude", "{0} s"),
			FText::AsNumber(ScalableFloatEventMagnitude.Value));

	case EGameplayEffectEventMagnitude::SetByCaller:
		return NSLOCTEXT("GameplayEffect", "SetByCallerModifierMagnitude", "Set by Caller");
	}

	return NSLOCTEXT("GameplayEffect", "UnknownModifierMagnitude", "Unknown");
}
#endif // WITH_EDITOR
