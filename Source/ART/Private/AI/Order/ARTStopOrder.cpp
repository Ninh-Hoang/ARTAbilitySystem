// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTStopOrder.h"
#include "GameFramework/Actor.h"
#include "Ability/ARTGlobalTags.h"


UARTStopOrder::UARTStopOrder()
{
	TargetType = EARTTargetType::NONE;
	bIsCreatingIndividualTargetLocations = false;

	//TagRequirements.SourceBlockedTags.AddTag(UARTGlobalTags::Status_Changing_Constructing());
}

bool UARTStopOrder::AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const 
{
	return true;
}