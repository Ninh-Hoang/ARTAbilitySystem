// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "ARTGameplayAbilitySet.generated.h"

/**
 * 
 */

class UARTAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FARTAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UARTGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly)
	int32 EffectLevel;
};

USTRUCT(BlueprintType)
struct FARTAbilitySet_Ability
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UARTGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FARTAbilitySetHandle
{
	GENERATED_BODY()
     
	bool IsValid() const
	{
		return AbilitySystemComponent.IsValid() && HandleId != 0;
	}
 
private:
	friend class UARTAbilitySet;
 
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
 
     
	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
 
	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
 
 
     
	int32 HandleId = 0;
     
	TWeakObjectPtr<UARTAbilitySystemComponent> AbilitySystemComponent = nullptr;
     
	void Reset()
	{
		AbilitySpecHandles.Reset();
		GameplayEffectHandles.Reset();
		AbilitySystemComponent.Reset();
		HandleId = 0;
	}
     
};

UCLASS()
class ART_API UARTAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
 
	UARTAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
 
	const TArray<FARTAbilitySet_Ability>& GetGrantedGameplayAbilities() const { return GrantedGameplayAbilities; }
	const TArray<FARTAbilitySet_GameplayEffect>& GetGrantedGameplayEffects() const { return GrantedGameplayEffects; }
 
	FARTAbilitySetHandle GiveAbilitySetTo(UARTAbilitySystemComponent* ASC, UObject* OverrideSourceObject = nullptr) const;
	FARTAbilitySetHandle GiveAbilitySetToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UObject* OverrideSourceObject = nullptr) const;
	static void TakeAbilitySet(FARTAbilitySetHandle& AbilitySetHandle);
 
protected:
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FARTAbilitySet_Ability> GrantedGameplayAbilities;
 
	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FARTAbilitySet_GameplayEffect> GrantedGameplayEffects;
};
