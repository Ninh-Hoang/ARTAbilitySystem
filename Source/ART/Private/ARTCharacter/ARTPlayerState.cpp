// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTPlayerState.h"
#include "Ability/ARTAbilitySystemComponent.h"

#include "ARTCharacter/ARTPlayerController.h"
#include "Widget/ARTHUDWidget.h"

#include "Inventory/Component/ARTInventoryComponent.h"
#include "Inventory/InventorySet.h"

#include "ARTCharacter/ARTCharacterBase.h"
#include "Ability/AttributeSet/ARTAttributeSet_Health.h"
#include "Inventory/Component/ARTInventoryComponent_Active.h"

FName AARTPlayerState::AbilitySystemComponentName(TEXT("AbilitySystemComponent"));
FName AARTPlayerState::InventoryComponentName(TEXT("InventoryComponent"));

AARTPlayerState::AARTPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UARTAbilitySystemComponent>(AbilitySystemComponentName);
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	InventoryComponent = CreateDefaultSubobject<UARTInventoryComponent_Active>(InventoryComponentName);
	InventoryComponent->SetIsReplicated(true);
	
	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	NetUpdateFrequency = 60.0f;

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	//KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
}

class UAbilitySystemComponent* AARTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


class UARTInventoryComponent* AARTPlayerState::GetInventoryComponent() const
{
	return InventoryComponent;
}

bool AARTPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void AARTPlayerState::ShowAbilityConfirmPrompt(bool bShowPrompt)
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(GetOwner());
	if (PC)
	{
		UARTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmPrompt(bShowPrompt);
		}
	}
}

void AARTPlayerState::ShowInteractionPrompt(float InteractionDuration)
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(GetOwner());
	if (PC)
	{
		UARTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowInteractionPrompt(InteractionDuration);
		}
	}
}

void AARTPlayerState::HideInteractionPrompt()
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(GetOwner());
	if (PC)
	{
		UARTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->HideInteractionPrompt();
		}
	}
}

void AARTPlayerState::StartInteractionTimer(float InteractionDuration)
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(GetOwner());
	if (PC)
	{
		UARTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StartInteractionTimer(InteractionDuration);
		}
	}
}

void AARTPlayerState::StopInteractionTimer()
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(GetOwner());
	if (PC)
	{
		UARTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StopInteractionTimer();
		}
	}
}

int32 AARTPlayerState::GetCharacterLevel() const
{
	return 1;
}

float AARTPlayerState::GetHealth() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->GetNumericAttribute(UARTAttributeSet_Health::GetHealthAttribute());
	}

	return -1.f;
}

void AARTPlayerState::SetHealth(const float Health)
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->SetNumericAttributeBase(UARTAttributeSet_Health::GetHealthAttribute(), Health);
	}
}

void AARTPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->
		                              GetGameplayAttributeValueChangeDelegate(UARTAttributeSet_Health::GetHealthAttribute()).
		                              AddUObject(this, &AARTPlayerState::HealthChanged);
		
	}
}

void AARTPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerPawn)
	{
		PlayerPawn = Cast<AARTCharacterBase>(GetPawn());
	}

	// Check for and handle knockdown and death
	if (IsValid(PlayerPawn) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		//PlayerPawn->Die();
	}
}


void AARTPlayerState::InitializeStartInventory()
{
	if (InventorySet)
	{
		//InventorySet->InitInventory(InventoryComponent);
	}
}
