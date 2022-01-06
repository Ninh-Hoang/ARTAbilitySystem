// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipment.h"
#include "Weapon.generated.h"

/**
 * 
 */
class UARTGameplayAbility;
class UAbilitySystemComponent;
class UARTAbilitySystemComponent;
class UAnimMontage;
class UAnimInstance;
class AARTCharacterBase;

UENUM()
enum class EWeaponType : uint8
{
	WT_Range UMETA(DisplayName = "MeleeWeapon"),
	WT_Melee UMETA(DisplayName = "RangeWeapon"),
};

UENUM()
enum class EWeaponReadySlot: uint8
{
	WRS_RightHandGun UMETA(DisplayName = "GunSocket"),
	WRS_RightHandMelee UMETA(DisplayName = "MeleeSocket"),
};


UCLASS(Abstract, Blueprintable, BlueprintType)
class ART_API AWeapon : public AEquipment
{
	GENERATED_BODY()
public:
	AWeapon();

	~AWeapon();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Weapon")
	FGameplayTag WeaponTag;
protected:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|UI")
	TSubclassOf<class UARTHUDReticle> PrimaryHUDReticleClass;

	UPROPERTY(EditDefaultsOnly, Category = "ART|Weapon")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "ART|Weapon")
	EWeaponReadySlot WeaponReadySlot;

	UPROPERTY(EditDefaultsOnly)
	float ThreatValue;

	virtual void ThreatTrace();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Animation")
	TSubclassOf<UAnimInstance> AnimSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Animation")
	UAnimMontage* EquipWeaponMontage;

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;
	FGameplayTag WeaponIsFiringTag;

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "ART|Animation")
	UAnimMontage* GetEquipWeaponMontage() const;

	UFUNCTION(BlueprintCallable, Category = "ART|Weapon")
	TSubclassOf<class UARTHUDReticle> GetPrimaryHUDReticleClass() const;

	UFUNCTION(BlueprintCallable)
	virtual void EquipWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void UnEquipWeapon();
};
