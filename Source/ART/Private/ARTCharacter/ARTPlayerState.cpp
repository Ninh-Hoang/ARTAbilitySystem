// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTPlayerState.h"
#include "Ability/ARTAbilitySystemComponent.h"

#include "ARTCharacter/ARTPlayerController.h"
#include "Widget/ARTHUDWidget.h"

#include "Item/InventoryComponent.h"
#include "Item/InventorySet.h"

#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Health.h"

AARTPlayerState::AARTPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UARTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UARTAttributeSetBase>(TEXT("Attribute"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory Component"));
	InventoryComponent->SetIsReplicated(true);

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	//KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
}

class UAbilitySystemComponent* AARTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

class UARTAttributeSetBase* AARTPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

class UInventoryComponent* AARTPlayerState::GetInventoryComponent() const
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

bool AARTPlayerState::GetCooldownRemainingForTag(FGameplayTagContainer CooldownTags, float& TimeRemaining,
                                                 float& CooldownDuration)
{
	if (AbilitySystemComponent && CooldownTags.Num() > 0)
	{
		TimeRemaining = 0.f;
		CooldownDuration = 0.f;

		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
		TArray<TPair<float, float>> DurationAndTimeRemaining = AbilitySystemComponent->
			GetActiveEffectsTimeRemainingAndDuration(Query);
		if (DurationAndTimeRemaining.Num() > 0)
		{
			int32 BestIdx = 0;
			float LongestTime = DurationAndTimeRemaining[0].Key;
			for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
			{
				if (DurationAndTimeRemaining[Idx].Key > LongestTime)
				{
					LongestTime = DurationAndTimeRemaining[Idx].Key;
					BestIdx = Idx;
				}
			}

			TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

			return true;
		}
	}

	return false;
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

void AARTPlayerState::SetHealth(float Health)
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

	if (InventoryComponent)
	{
		InitializeStartInventory();
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
		InventorySet->InitInventory(InventoryComponent);
	}
}
