// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTPlayerState.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ARTCharacter/ARTCharacterAttributeSet.h"
#include <ARTCharacter/ARTSurvivor.h>
#include <Ability/ARTAbilitySystemGlobals.h>
#include <ARTCharacter/ARTPlayerController.h>
#include <Widget/ARTHUDWidget.h>
#include <Item/InventoryComponent.h>
#include <Item/InventorySet.h>

AARTPlayerState::AARTPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UARTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSetBase = CreateDefaultSubobject<UARTCharacterAttributeSet>(TEXT("AttributeSetBase"));

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

class UARTCharacterAttributeSet* AARTPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
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

float AARTPlayerState::GetAttackPower() const
{
	return AttributeSetBase->GetAttackPower();
}

float AARTPlayerState::GetCritRate() const
{
	return AttributeSetBase->GetCritRate();
}

float AARTPlayerState::GetCritMultiplier() const
{
	return AttributeSetBase->GetCritMultiplier();
}

float AARTPlayerState::GetReactMas() const
{
	return AttributeSetBase->GetReactMas();
}

float AARTPlayerState::GetPhysBonus() const
{
	return AttributeSetBase->GetPhysBonus();
}

float AARTPlayerState::GetPhysRes() const
{
	return AttributeSetBase->GetPhysRes();
}

float AARTPlayerState::GetArmor() const
{
	return AttributeSetBase->GetArmor();
}

float AARTPlayerState::GetHealBonus() const
{
	return AttributeSetBase->GetHealBonus();
}

float AARTPlayerState::GetIncomingHealBonus() const
{
	return AttributeSetBase->GetIncomingHealBonus();
}

float AARTPlayerState::GetVoidBonus() const
{
	return AttributeSetBase->GetVoidBonus();
}

float AARTPlayerState::GetVoidRes() const
{
	return AttributeSetBase->GetVoidRes();
}

float AARTPlayerState::GetHeatBonus() const
{
	return AttributeSetBase->GetHeatBonus();
}

float AARTPlayerState::GetHeatRes() const
{
	return AttributeSetBase->GetHeatRes();
}

float AARTPlayerState::GetColdBonus() const
{
	return AttributeSetBase->GetColdBonus();
}

float AARTPlayerState::GetColdRes() const
{
	return AttributeSetBase->GetColdRes();
}

float AARTPlayerState::GetElecBonus() const
{
	return AttributeSetBase->GetElecBonus();
}

float AARTPlayerState::GetElecRes() const
{
	return AttributeSetBase->GetElecRes();
}

float AARTPlayerState::GetWaterBonus() const
{
	return AttributeSetBase->GetWaterBonus();
}

float AARTPlayerState::GetWaterRes() const
{
	return AttributeSetBase->GetWaterRes();
}

float AARTPlayerState::GetEarthBonus() const
{
	return AttributeSetBase->GetEarthBonus();
}

float AARTPlayerState::GetEarthRes() const
{
	return AttributeSetBase->GetEarthRes();
}

float AARTPlayerState::GetAirBonus() const
{
	return AttributeSetBase->GetAirBonus();
}

float AARTPlayerState::GetAirRes() const
{
	return AttributeSetBase->GetAirRes();
}

float AARTPlayerState::GetLifeBonus() const
{
	return AttributeSetBase->GetLifeBonus();
}

float AARTPlayerState::GetLifeRes() const
{
	return AttributeSetBase->GetLifeRes();
}

float AARTPlayerState::GetShield() const
{
	return AttributeSetBase->GetShield();
}

float AARTPlayerState::GetMaxShield() const
{
	return AttributeSetBase->GetMaxShield();
}

float AARTPlayerState::GetShieldRegen() const
{
	return AttributeSetBase->GetShieldRegen();
}

float AARTPlayerState::GetHealth() const
{
	return AttributeSetBase->GetHealth();
}

float AARTPlayerState::GetMaxHealth() const
{
	return AttributeSetBase->GetMaxHealth();
}

float AARTPlayerState::GetHealthRegen() const
{
	return AttributeSetBase->GetHealthRegen();
}

float AARTPlayerState::GetPartHealthA() const
{
	return AttributeSetBase->GetPartHealthA();
}

float AARTPlayerState::GetPartHealthB() const
{
	return AttributeSetBase->GetPartHealthB();
}

float AARTPlayerState::GetPartHealthC() const
{
	return AttributeSetBase->GetPartHealthC();
}

float AARTPlayerState::GetPartHealthD() const
{
	return AttributeSetBase->GetPartHealthD();
}

float AARTPlayerState::GetPartHealthE() const
{
	return AttributeSetBase->GetPartHealthE();
}

float AARTPlayerState::GetPartHealthF() const
{
	return AttributeSetBase->GetPartHealthF();
}

float AARTPlayerState::GetEnergy() const
{
	return AttributeSetBase->GetEnergy();
}

float AARTPlayerState::GetMaxEnergy() const
{
	return AttributeSetBase->GetMaxEnergy();
}

float AARTPlayerState::GetEnergyRegen() const
{
	return AttributeSetBase->GetEnergyRegen();
}

float AARTPlayerState::GetStamina() const
{
	return AttributeSetBase->GetStamina();
}

float AARTPlayerState::GetMaxStamina() const
{
	return AttributeSetBase->GetMaxStamina();
}

float AARTPlayerState::GetStaminaRegen() const
{
	return AttributeSetBase->GetStaminaRegen();
}

float AARTPlayerState::GetMoveSpeed() const
{
	return AttributeSetBase->GetMoveSpeed();
}

float AARTPlayerState::GetRotateRate() const
{
	return AttributeSetBase->GetRotateRate();
}

float AARTPlayerState::GetXPMod() const
{
	return AttributeSetBase->GetXPMod();
}

float AARTPlayerState::GetEnMod() const
{
	return AttributeSetBase->GetEnMod();
}


void AARTPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->
		                              GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).
		                              AddUObject(this, &AARTPlayerState::HealthChanged);
	}

	if (InventoryComponent)
	{
		InitializeStartInventory();
	}
}

void AARTPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	if (!Survivor)
	{
		Survivor = Cast<AARTSurvivor>(GetPawn());
	}

	// Check for and handle knockdown and death
	if (IsValid(Survivor) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		//Survivor->Die();
	}
}


void AARTPlayerState::InitializeStartInventory()
{
	if (InventorySet)
	{
		InventorySet->InitInventory(InventoryComponent);
	}
}
