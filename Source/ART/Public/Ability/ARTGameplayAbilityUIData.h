// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameplayEffectUIData.h"
#include <GameplayTagContainer.h>
#include "ARTGameplayAbilityUIData.generated.h"

/**
 * UGameplayAbilityUIData
 * Base class to provide game-specific data about how to describe a Gameplay Ability in the UI. Subclass with data to use in your game.
 */

UCLASS()
class ART_API UARTGameplayAbilityUIData : public UGameplayEffectUIData
{
	GENERATED_UCLASS_BODY()

	/** Icon of this ability. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data)
	UTexture2D* Icon;

	/** Name of this ability. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data)
	FText AbilityName;

	/** Description of this ability. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data, meta = (MultiLine = "true"))
	FText Description;

	/** Description of this ability. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data, meta = (MultiLine = "true"))
	FGameplayTag CooldownTag;
};
