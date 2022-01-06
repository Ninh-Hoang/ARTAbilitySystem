// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "ARTGlobalTags.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTGlobalTags : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// ---------------------------------------------------------------------------------------------------
	// Behaviour tags
	// ---------------------------------------------------------------------------------------------------

	/** Behaviour parent tag. */
	static const FGameplayTag& Behaviour();

	/** Whether the two actor references resolve to the same actor. */
	static const FGameplayTag& Behaviour_Self();

	/** Whether the Behaviour to this actor is friendly. */
	static const FGameplayTag& Behaviour_Friendly();

	/** Whether the Behaviour to this actor is hostile. */
	static const FGameplayTag& Behaviour_Hostile();

	/** Whether the Behaviour to this actor is neutral. */
	static const FGameplayTag& Behaviour_Neutral();

	/** Whether the actor is visible. */
	static const FGameplayTag& Behaviour_Visible();

	// ---------------------------------------------------------------------------------------------------
	// Ability activation failure tags
	// ---------------------------------------------------------------------------------------------------

	/** The ability cannot be activated because of a blocking tag. */
	static const FGameplayTag& AbilityActivationFailure_BlockedTag();

	/** The ability cannot be activated because of a required tag. */
	static const FGameplayTag& AbilityActivationFailure_MissingTag();

	/** The ability cannot be activated because it is still on cooldown. */
	static const FGameplayTag& AbilityActivationFailure_Cooldown();

	/** The ability cannot be activated because the required costs cannot be payed. */
	static const FGameplayTag& AbilityActivationFailure_Cost();

	/** The ability cannot be activated because no target has been specified. */
	static const FGameplayTag& AbilityActivationFailure_NoTarget();
};
