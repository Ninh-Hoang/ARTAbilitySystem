// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ARTHUDWidget.generated.h"

/**
 * 
 */

class UPaperSprite;
class UTexture2D;

UCLASS()
class ART_API UARTHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowAbilityConfirmPrompt(bool bShowText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetRespawnCountdown(float RespawnTimeRemaining);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowInteractionPrompt(float InteractionDuration);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideInteractionPrompt();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StartInteractionTimer(float InteractionDuration);

	// Interaction interrupted, cancel and hide HUD interact timer
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StopInteractionTimer();


	/**
	* Weapon info
	*/

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponSprite(UPaperSprite* Sprite);

	// Things like fire mode for rifle
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponStatusText(const FText& StatusText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryClipAmmo(int32 ClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryReserveAmmo(int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryClipAmmo(int32 SecondaryClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetReticle(TSubclassOf<class UARTHUDReticle> ReticleClass);


	/**
	* Attribute setters
	*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxHealth(float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentHealth(float CurrentHealth);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthRegen(float HealthRegen);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxStamina(float MaxStamina);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentStamina(float CurrentStamina);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetStaminaPercentage(float StaminaPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetStaminaRegen(float StaminaRegen);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxEnergy(float MaxEnergy);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentEnergy(float CurrentEnergy);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEnergyPercentage(float EnergyPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEnergyRegen(float EnergyRegen);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxShield(float MaxShield);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentShield(float CurrentShield);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetShieldPercentage(float ShieldPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetShieldRegen(float ShieldRegen);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetExperience(int32 Experience);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCharacterLevel(int32 HeroLevel);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEn(float En);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMoveSpeed(float MoveSpeed);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAttackPower(float AttackPower);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCritRate(float CritRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCritMultiplier(float CritMultiplier);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetReactMas(float ReactMas);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetArmor(float Armor);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPhysBonus(float PhysBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPhysRes(float PhysRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealBonus(float HealBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetIncomingHealBonus(float IncomingHealBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetVoidBonus(float VoidBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetVoidRes(float VoidRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHeatBonus(float HeatBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHeatRes(float HeatRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColdBonus(float ColdBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColdRes(float ColdRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetElecBonus(float ElecBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetElecRes(float ElecRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetWaterBonus(float WaterBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetWaterRes(float WaterRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEarthBonus(float EarthBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEarthRes(float EarthRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAirBonus(float AirBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAirRes(float AirRes);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetLifeBonus(float LifeBonus);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetLifeRes(float LifeRes);
};
