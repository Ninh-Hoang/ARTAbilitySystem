// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTActorBase.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ARTCharacter/ARTGameplayAbilitySet.h"

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

UAbilitySystemComponent* AARTActorBase::GetAbilitySystemComponent() const
{
	//return if already valid (not lazy loading)
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AARTActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AARTActorBase::CreateASC()
{
	AbilitySystemComponent = NewObject<UARTAbilitySystemComponent>(this);
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AbilitySystemComponent->RegisterComponent();
}

void AARTActorBase::InitializeASC()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
    
	//TODO: Level
	if (HasAuthority())
	{
		for (TSoftObjectPtr<UARTAbilitySet> Set : AbilitySets)
		{
			UARTAbilitySet* LoadedSet = Set.IsValid() ? Set.Get() : Set.LoadSynchronous();
			if (LoadedSet)
			{
				LoadedSet->GiveAbilitySetTo(AbilitySystemComponent, this);
			}
		}
	}
}
