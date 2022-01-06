// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "RangeWeapon.generated.h"

/**
 * 
 */
//range weapon type

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

UENUM()
enum class ERangeWeaponType : uint8
{
	WT_Tracer UMETA(DisplayName = "TracerRangeWeapon"),
	WT_Projectile UMETA(DisplayName = "ProjectileRangeWeapon"),
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class ART_API ARangeWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	ARangeWeapon();

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|GSWeapon")
	FGameplayTag SecondaryAmmoType;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|GSWeapon")
	FWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSWeapon")
	virtual bool HasInfiniteAmmo() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	ERangeWeaponType RangeWeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	bool bAutomatic;
	//RPM
	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float Range;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float FirstShotDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float FiringRotationSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float AimRotationSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float FiringMovementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Range Weapon")
	float AimMovementSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName MuzzleSocketName;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category =
		"GASShooter|GSWeapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category =
		"GASShooter|GSWeapon|Ammo")
	int32 MaxPrimaryClipAmmo;

	// How much ammo in the clip the gun starts with. Used for things like rifle grenades.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo, Category =
		"GASShooter|GSWeapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo, Category =
		"GASShooter|GSWeapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|GSWeapon|Ammo")
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|GSWeapon")
	FGameplayTag DefaultFireMode;

	FTimerHandle TimerHandle_TimeBetweenShot;

	//derived from RateOfFire
	float TimeBetweenShot;

	float LastFireTime;

	float OwnerRotationSpeed;
	float OwnerMovementSpeed;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);
};
