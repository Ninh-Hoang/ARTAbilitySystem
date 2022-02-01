// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagsManager.h"

/**
 * 
 */
struct ART_API FARTGlobalTags : public FGameplayTagNativeAdder
{
	
public:
	// ---------------------------------------------------------------------------------------------------
	// Behaviour tags
	// ---------------------------------------------------------------------------------------------------

	/** Whether the two actor references resolve to the same actor. */
	FGameplayTag Behaviour_Self;

	/** Whether the Behaviour to this actor is friendly. */
	FGameplayTag Behaviour_Friendly;

	/** Whether the Behaviour to this actor is hostile. */
	FGameplayTag Behaviour_Hostile;

	/** Whether the Behaviour to this actor is neutral. */
	FGameplayTag Behaviour_Neutral;

	/** Whether the actor is visible. */
	FGameplayTag Behaviour_Visible;

	// ---------------------------------------------------------------------------------------------------
	// Ability activation failure tags
	// ---------------------------------------------------------------------------------------------------

	/** The ability cannot be activated because of a blocking tag. */
	FGameplayTag AbilityActivationFailure_BlockedTag;

	/** The ability cannot be activated because of a required tag. */
	FGameplayTag AbilityActivationFailure_MissingTag;

	/** The ability cannot be activated because it is still on cooldown. */
	FGameplayTag AbilityActivationFailure_Cooldown;

	/** The ability cannot be activated because the required costs cannot be payed. */
	FGameplayTag AbilityActivationFailure_Cost;

	/** The ability cannot be activated because no target has been specified. */
	FGameplayTag AbilityActivationFailure_NoTarget;

	FORCEINLINE static const FARTGlobalTags& Get() { return ARTTags; }

protected:
	//Called to register and assign the native tags
	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
 
		Behaviour_Self = Manager.AddNativeGameplayTag(TEXT("Behaviour.Self"));
		Behaviour_Friendly = Manager.AddNativeGameplayTag(TEXT("Behaviour.Friendly"));
		Behaviour_Hostile = Manager.AddNativeGameplayTag(TEXT("Behaviour.Hostile"));
		Behaviour_Neutral = Manager.AddNativeGameplayTag(TEXT("Behaviour.Neutral"));
		Behaviour_Visible = Manager.AddNativeGameplayTag(TEXT("Behaviour.Visible"));

		AbilityActivationFailure_BlockedTag = Manager.AddNativeGameplayTag(TEXT("AbilityActivationFailure.BlockedTag"));
		AbilityActivationFailure_MissingTag = Manager.AddNativeGameplayTag(TEXT("AbilityActivationFailure.MissingTag"));
		AbilityActivationFailure_Cooldown = Manager.AddNativeGameplayTag(TEXT("AbilityActivationFailure.Cooldown"));
		AbilityActivationFailure_Cost = Manager.AddNativeGameplayTag(TEXT("AbilityActivationFailure_Cost"));
		AbilityActivationFailure_NoTarget = Manager.AddNativeGameplayTag(TEXT("AbilityActivationFailure.NoTarget"));
	}

private:
	static FARTGlobalTags ARTTags;
};
