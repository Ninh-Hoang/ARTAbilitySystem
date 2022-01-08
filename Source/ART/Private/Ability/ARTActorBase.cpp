// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTActorBase.h"
#include <Ability/ARTAbilitySystemComponent.h>

// Sets default values
AARTActorBase::AARTActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UARTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Minimal mode means GameplayEffects are not replicated to anyone. Only GameplayTags and Attributes are replicated to clients.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
}

class UAbilitySystemComponent* AARTActorBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AARTActorBase::BeginPlay()
{
	Super::BeginPlay();
}
