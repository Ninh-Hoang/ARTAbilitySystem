// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "AbilitySystemInterface.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "ARTItemDefinition_Mod.generated.h"

/**
 * 
 */

class UGameplayEffect;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FARTModProperty_Attribute
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> Attribute;
};

USTRUCT(BlueprintType)
struct FARTModProperty_Ability
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FARTModProperty_GameplayEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly)
	int32 EffectLevel;
};

USTRUCT(BlueprintType)
struct FARTModPropertyHandle
{
	GENERATED_BODY()
     
	bool IsValid() const
	{
		return AbilitySystemComponent.IsValid() && HandleId != 0;
	}
 
private:
	friend class UARTItemDefinition_Mod;
	
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	
	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
 
	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
	
	int32 HandleId = 0;
     
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
     
	void Reset()
	{
		AbilitySpecHandles.Reset();
		GameplayEffectHandles.Reset();
		AbilitySystemComponent.Reset();
		HandleId = 0;
	}

	bool operator==(const FARTModPropertyHandle& Other) const
	{
		return HandleId == Other.HandleId;
	}

	bool operator!=(const FARTModPropertyHandle& Other) const
	{
		return HandleId != Other.HandleId;
	}

	friend uint32 GetTypeHash(const FARTModPropertyHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.HandleId);
	}

	FString ToString() const
	{
		return IsValid() ? FString::FromInt(HandleId) : TEXT("Invalid");
	}
};

UCLASS(Abstract)
class ART_API UARTItemDefinition_Mod : public UARTItemDefinition
{
	GENERATED_BODY()
public:
	UARTItemDefinition_Mod();

	const TArray<FARTModProperty_Attribute>& GetGrantedAttributeSets() const { return GrantedAttributeSets; }
	const TArray<FARTModProperty_Ability>& GetGrantedGameplayAbilities() const { return GrantedGameplayAbilities; }
	const TArray<FARTModProperty_GameplayEffect>& GetGrantedGameplayEffects() const { return GrantedGameplayEffects; }
 
	FARTModPropertyHandle GiveModPropertyTo(UAbilitySystemComponent* ASC, UObject* OverrideSourceObject = nullptr) const;
	FARTModPropertyHandle GiveModPropertyToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UObject* OverrideSourceObject = nullptr) const;
	static void TakeModProperty(FARTModPropertyHandle& ModPropertyHandle);
	
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Mod | Gameplay Attribute", meta=(TitleProperty=Attribute))
	TArray<FARTModProperty_Attribute> GrantedAttributeSets;

	UPROPERTY(EditDefaultsOnly, Category = "Mod | Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FARTModProperty_Ability> GrantedGameplayAbilities;
	
	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Mod | Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FARTModProperty_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mod | Application Requirement")
	FGameplayTagQuery ParentStackApplicationRequirement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mod | Application Requirement")
	FGameplayTagQuery SlotApplicationRequirement;
};
