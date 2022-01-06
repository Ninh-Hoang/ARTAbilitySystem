// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "ARTCharacter/ARTCharacterBase.h"

AWeapon::AWeapon()
{
	WeaponType = EWeaponType::WT_Range;
	WeaponReadySlot = EWeaponReadySlot::WRS_RightHandGun;

	ThreatValue = 0;

	//WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Primary.Instant"));
	//WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Secondary.Instant"));
	//WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Alternate.Instant"));
	//WeaponIsFiringTag = FGameplayTag::RequestGameplayTag(FName("Weapon.IsFiring"));
}

AWeapon::~AWeapon()
{
}

UAnimMontage* AWeapon::GetEquipWeaponMontage() const
{
	return EquipWeaponMontage;
}

TSubclassOf<class UARTHUDReticle> AWeapon::GetPrimaryHUDReticleClass() const
{
	return PrimaryHUDReticleClass;
}

void AWeapon::EquipWeapon()
{
	if (OwningCharacter)
	{
		FText SocketText = UEnum::GetDisplayValueAsText(WeaponReadySlot);
		FName SocketName = FName(*SocketText.ToString());

		AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                  SocketName);
		if (AnimSet)
		{
			OwningCharacter->GetMesh()->SetAnimInstanceClass(AnimSet);
		}
	}
}

void AWeapon::UnEquipWeapon()
{
	if (OwningCharacter)
	{
		FText SocketText = UEnum::GetDisplayValueAsText(EquipmentSlot);
		FName SocketName = FName(*SocketText.ToString());
		AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                  SocketName);
	}
}

void AWeapon::ThreatTrace()
{
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}
