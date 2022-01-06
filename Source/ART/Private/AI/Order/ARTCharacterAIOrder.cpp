// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTCharacterAIOrder.h"

UARTCharacterAIOrder::UARTCharacterAIOrder()
{
	TargetType = EARTTargetType::NONE;
	bIsCreatingIndividualTargetLocations = false;

	GroupExecutionType = EARTOrderGroupExecutionType::ALL;
}

EARTTargetType UARTCharacterAIOrder::GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return TargetType;
}

bool UARTCharacterAIOrder::IsCreatingIndividualTargetLocations(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return bIsCreatingIndividualTargetLocations;
}

EARTOrderGroupExecutionType UARTCharacterAIOrder::GetGroupExecutionType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const
{
	return GroupExecutionType;
}