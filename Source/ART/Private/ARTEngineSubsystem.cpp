// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTEngineSubsystem.h"
#include "AbilitySystemGlobals.h"

void UARTEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAbilitySystemGlobals::Get().InitGlobalData();
}
