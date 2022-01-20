// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ARTTargetType.h"

#include "Ability/ARTGameplayEffectTypes.h"
#include "ARTCharacter/ARTCharacterBase.h"

void UARTTargetType::GetTargets_Implementation(AARTCharacterBase* TargetingCharacter, AActor* TargetingActor,
                                               FGameplayEventData EventData,
                                               TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                               TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
}

void UARTTargetType_UseOwner::GetTargets_Implementation(AARTCharacterBase* TargetingCharacter, AActor* TargetingActor,
                                                        FGameplayEventData EventData,
                                                        TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                                        TArray<FHitResult>& OutHitResults,
                                                        TArray<AActor*>& OutActors) const
{
	OutActors.Add(TargetingCharacter);
}

void UARTTargetType_UseEventData::GetTargets_Implementation(AARTCharacterBase* TargetingCharacter,
                                                            AActor* TargetingActor, FGameplayEventData EventData,
                                                            TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                                            TArray<FHitResult>& OutHitResults,
                                                            TArray<AActor*>& OutActors) const
{
	const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult();
	if (FoundHitResult)
	{
		OutHitResults.Add(*FoundHitResult);
	}
	else if (EventData.Target)
	{
		OutActors.Add(const_cast<AActor*>(EventData.Target));
	}
}
