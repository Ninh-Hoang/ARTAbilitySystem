// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ARTAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	/**
	* Cache commonly used tags here. This has the benefit of one place to set the tag FName in case tag names change and
	* the function call into UGSAbilitySystemGlobals::GSGet() is cheaper than calling FGameplayTag::RequestGameplayTag().
	* Classes can access them by UARTAbilitySystemGlobals::GSGet().DeadTag
	*/

	UARTAbilitySystemGlobals();

	UPROPERTY()
	FGameplayTag DeadTag;

	UPROPERTY()
	FGameplayTag KnockedDownTag;

	UPROPERTY()
	FGameplayTag InteractingTag;

	UPROPERTY()
	FGameplayTag InteractingRemovalTag;

	static UARTAbilitySystemGlobals& GetARTASG()
	{
		return dynamic_cast<UARTAbilitySystemGlobals&>(Get());
	}
	
	virtual void InitGlobalTags() override;
	
	/** Should allocate a project specific GameplayEffectContext struct. Caller is responsible for deallocation */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	/** Should allocate a project specific GameplayAbilityActorInfo struct. Caller is responsible for deallocation */
	virtual FGameplayAbilityActorInfo* AllocAbilityActorInfo() const override;
};
