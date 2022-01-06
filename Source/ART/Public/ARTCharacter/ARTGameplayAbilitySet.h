// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTGameplayAbilitySet.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FARTGameplayEffectApplicationInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UARTGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Level;
};

USTRUCT(BlueprintType)
struct FARTGameplayAbilityApplicationInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UARTGameplayAbility> GameplayAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Level;
};

UCLASS()
class ART_API UARTGameplayAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FARTGameplayAbilityApplicationInfo> StartupGameplayAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FARTGameplayEffectApplicationInfo> StartupGameplayEffects;

	void GiveAbilities(class UARTAbilitySystemComponent* AbilitySystemComponent) const;

	void AddStartupEffects(class UARTAbilitySystemComponent* AbilitySystemComponent) const;
};
