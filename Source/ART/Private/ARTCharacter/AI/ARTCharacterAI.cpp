// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/AI/ARTCharacterAI.h"
#include <Ability/ARTAbilitySystemComponent.h>
#include <ARTCharacter/ARTCharacterAttributeSet.h>
#include <Components/CapsuleComponent.h>
#include <Components/WidgetComponent.h>
#include <Widget/ARTFloatingStatusBarWidget.h>

#include "AI/ARTAIConductor.h"
#include "AI/Order/ARTAutoOrderComponent.h"
#include "AI/Order/ARTOrderComponent.h"
#include "AI/Order/ARTSelectComponent.h"
#include "ARTCharacter/AI/ARTNavigationInvokerComponent.h"
#include "Framework/ARTGameState.h"


AARTCharacterAI::AARTCharacterAI(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//set auto possess
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	//Create Nav Invoker
	NavInvoker = CreateDefaultSubobject<UARTNavigationInvokerComponent>(TEXT("NavInvokerComp"));

	//Create Ability System Component
	HardRefAbilitySystemComponent = CreateDefaultSubobject<UARTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HardRefAbilitySystemComponent->SetIsReplicated(true);

	// Minimal Mode means that no GameplayEffects will replicate. They will only live on the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	HardRefAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Set our parent's TWeakObjectPtr
	AbilitySystemComponent = HardRefAbilitySystemComponent;

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	if(AttributeSetClass)
	{
		HardRefAttributeSetBase = Cast<UARTCharacterAttributeSet>(CreateDefaultSubobject(TEXT("AttributeSetBase"), AttributeSetClass, AttributeSetClass, true,  false));
	}
	else
	{
		HardRefAttributeSetBase = CreateDefaultSubobject<UARTCharacterAttributeSet>(TEXT("AttributeSetBase"));
	}
	
	// Set our parent's TWeakObjectPtr
	AttributeSetBase = HardRefAttributeSetBase;

	//selectcomp
	SelectComponent = CreateDefaultSubobject<UARTSelectComponent>(TEXT("SelectComponent"));
	
	//create order comps
	OrderComponent = CreateDefaultSubobject<UARTOrderComponent>(TEXT("OrderComponent"));
	AutoOrderComponent = CreateDefaultSubobject<UARTAutoOrderComponent>(TEXT("AutoOrderComponent"));
	
	//set collision
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	//affact navigation
	SetCanAffectNavigationGeneration(true);

	//not using controller Yaw, might want to change this
	bUseControllerRotationYaw = false;
}

void AARTCharacterAI::BeginPlay()
{	
	Super::BeginPlay();

	//Add AI to Director if it was not load from map (drop in map)
	if(!HasAnyFlags(RF_WasLoaded))
	{
		AARTGameState::GetAIConductor(this)->AddAIToList(this);
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAttributes();
		AddStartupEffects();
		AddCharacterAbilities();
		InitializeTagPropertyMap();
		InitializeTagResponseTable();

		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->
		                              GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).
		                              AddUObject(this, &AARTCharacterAI::HealthChanged);
	}

	// Setup FloatingStatusBar UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UARTFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());

				UIFloatingStatusBar->SetCharacterName(CharacterName);
			}
		}
	}
}

void AARTCharacterAI::FinishDying()
{
	//remove from AIManager
	if(HasAuthority()) AARTGameState::GetAIConductor(this)->RemoveAIFromList(this);
	Super::FinishDying();
}

void AARTCharacterAI::HealthChanged(const FOnAttributeChangeData& Data)
{
	float Health = Data.NewValue;

	// Update floating status bar
	if (UIFloatingStatusBar)
	{
		UIFloatingStatusBar->SetHealthPercentage(Health / GetMaxHealth());
	}

	// If the minion died, handle death
	if (!IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		Die();
	}
}
