// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTAttackOrder.h"
#include "ARTGameplayAbility_Order.h"
#include "Ability/ARTAbilitySystemComponent.h"

UARTAttackOrder::UARTAttackOrder()
{
}

bool UARTAttackOrder::CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
	FARTOrderErrorTags* OutErrorTags) const
{
	return true;
}

void UARTAttackOrder::CreateIndividualTargetLocations(const TArray<AActor*>& OrderedActors,
	const FARTOrderTargetData& TargetData, TArray<FVector2D>& OutTargetLocations) const
{
	Super::CreateIndividualTargetLocations(OrderedActors, TargetData, OutTargetLocations);
}

float UARTAttackOrder::GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	int32 Index) const
{
	return Super::GetRequiredRange(OrderedActor, OrderTags, Index);
}

float UARTAttackOrder::GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
}

bool UARTAttackOrder::AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const 
{
	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility_Order* Ability = Cast<UARTGameplayAbility_Order>(GetAbility(AbilitySystem, OrderTags));	
	if(Ability->AbilityTags.HasAll(OrderTags))
	{
		return false;
	}
	return true;
}


