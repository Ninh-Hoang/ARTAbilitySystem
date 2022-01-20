// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ARTGameplayEffect.generated.h"

/**
 * 
 */

/** Enum for sending gameplay event from source to target or target to souce. */
UENUM()
enum class EGameplayEffectEventDirection : uint8
{
	/** Source as instigator, Target as target. */
	SourceToTarget,
	/** Target as instigator, Source as target. */
	TargetToSource,
};

/** Enum for event magnitude setup. */
UENUM()
enum class EGameplayEffectEventMagnitude : uint8
{
	/** Use a simple, scalable float for the calculation. */
	ScalableFloat,
	/** This magnitude will be set explicitly by the code/blueprint that creates the spec. */
	SetByCaller,
};

//struct for setting up GE FGameplayEvent sending
USTRUCT()
struct ART_API FGameplayEffectEvent
{
	GENERATED_USTRUCT_BODY()

public:

	/** Default Constructor */
	FGameplayEffectEvent()
		: GameplayEventMagnitudeCalculation(EGameplayEffectEventMagnitude::ScalableFloat)
	{
	}

	/** Constructors for setting value in code (for automation tests) */
	FGameplayEffectEvent(const FScalableFloat& Value)
		: GameplayEventMagnitudeCalculation(EGameplayEffectEventMagnitude::ScalableFloat)
		  , ScalableFloatEventMagnitude(Value)
	{
	}

	FGameplayEffectEvent(const FSetByCallerFloat& Value)
		: GameplayEventMagnitudeCalculation(EGameplayEffectEventMagnitude::SetByCaller)
		  , SetByCallerEventMagnitude(Value)
	{
	}

	/**
	 * Attempts to calculate the magnitude given the provided spec. May fail if necessary information (such as captured attributes) is missing from
	 * the spec.
	 *
	 * @param InRelevantSpec			Gameplay effect spec to use to calculate the magnitude with
	 * @param OutCalculatedMagnitude	[OUT] Calculated value of the magnitude, will be set to 0.f in the event of failure
	 *
	 * @return True if the calculation was successful, false if it was not
	 */

	bool AttemptCalculateMagnitude(const FGameplayEffectSpec& InRelevantSpec, OUT float& OutCalculatedMagnitude,
	                               bool WarnIfSetByCallerFail = true, float DefaultSetbyCaller = 0.f) const;

	bool AttemptReturnGameplayEventTags(const FGameplayTagContainer* InstigatorTags,
	                                    const FGameplayTagContainer* TargetTags, OUT FGameplayTag& InEventTag,
	                                    OUT FGameplayTagContainer& InInstigatorTags,
	                                    OUT FGameplayTagContainer& InTargetTags);

	bool AttempAssignGameplayEventDataActors(AActor* SourceActor, AActor* TargetActor, OUT AActor*& Instigator,
	                                         OUT AActor*& Target);

	/** Returns the magnitude as it was entered in data. Only applies to ScalableFloat or any other type that can return data without context */
	bool GetStaticMagnitudeIfPossible(float InLevel, float& OutMagnitude, const FString* ContextString = nullptr) const;

	/** Returns the DataName associated with this magnitude if it is set by caller */
	bool GetSetByCallerDataNameIfPossible(FName& OutDataName) const;

	/** Returns SetByCaller data structure, for inspection purposes */
	const FSetByCallerFloat& GetSetByCallerFloat() const { return SetByCallerEventMagnitude; }

	/** Implementing Serialize to clear references to assets that are not needed */
	bool Serialize(FArchive& Ar);

	bool operator==(const FGameplayEffectEvent& Other) const;
	bool operator!=(const FGameplayEffectEvent& Other) const;

#if WITH_EDITOR
	FText GetValueForEditorDisplay() const;
#endif

protected:
	/** Direction of GameplayEvent */
	UPROPERTY(EditDefaultsOnly, Category = Category = "ART|GameplayEvent")
	EGameplayEffectEventDirection GameplayEventDirection;

	UPROPERTY(EditDefaultsOnly, Category = "ART|GameplayEvent")
	FGameplayTag GameplayEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "ART|GameplayEvent")
	EGameplayEffectEventMagnitude GameplayEventMagnitudeCalculation;

	/** Magnitude value represented by a scalable float */
	UPROPERTY(EditDefaultsOnly, Category = "ART|GameplayEvent")
	FScalableFloat ScalableFloatEventMagnitude;

	UPROPERTY(EditDefaultsOnly, Category = "ART|GameplayEvent")
	FSetByCallerFloat SetByCallerEventMagnitude;

	//friend class UGameplayEffect;
	//friend class FGameplayEffectModifierMagnitudeDetails;
};

UCLASS()
class ART_API UARTGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "ART")
	TSoftObjectPtr<class UARTCurve> Curves;

	UPROPERTY(EditDefaultsOnly, Category = "ART")
	TArray<FGameplayEffectEvent> GameplayEvents;

protected:
};
