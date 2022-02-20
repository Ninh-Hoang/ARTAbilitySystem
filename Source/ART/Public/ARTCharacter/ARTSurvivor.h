// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTSurvivor.generated.h"

/**
 * 
 */

class AWeapon;
class AEquipment;

USTRUCT()
struct ART_API FSurvivorEquipment
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<AWeapon*> Weapons;

	//equipment
	UPROPERTY()
	TArray<AEquipment*> Equipments;

	// Consumable items

	// Passive items like armor

	// Door keys

	// Etc
};

UCLASS()
class ART_API AARTSurvivor : public AARTCharacterBase
{
	GENERATED_BODY()

public:

	AARTSurvivor(const class FObjectInitializer& ObjectInitializer);

	//TAG STUFFS
	FGameplayTag CurrentWeaponTag;

	// Cache tags
	FGameplayTag NoWeaponTag;
	FGameplayTag WeaponChangingDelayReplicationTag;
	FGameplayTag WeaponAmmoTypeNoneTag;
	FGameplayTag WeaponAbilityTag;

	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* AzimuthComponent;
	
	//INITIALIZATION

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Client only
	virtual void OnRep_PlayerState() override;

	//DEAD STUFFS
public:
	virtual void Die() override;

	//Player unique UI STUFFS
public:
	class UARTFloatingStatusBarWidget* GetFloatingStatusBar();

protected:

	virtual void InitializeFloatingStatusBar();

	//EQUIPMENT LIST
protected:

	UPROPERTY(ReplicatedUsing = OnRep_Equipment)
	FSurvivorEquipment Equipment;

	UFUNCTION()
	void OnRep_Equipment();

	//WEAPON STUFFS
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeapon* GetCurrentWeapon() const;

protected:
	//current weapon
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AWeapon* CurrentWeapon;

	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);

	void SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerSyncCurrentWeapon();

	UFUNCTION(Client, WithValidation, Reliable)
	void ClientSyncCurrentWeapon(AWeapon* InWeapon);

	//equip new weapon
	bool bChangedWeaponLocally;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(AWeapon* NewWeapon);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEquipWeapon(AWeapon* NewWeapon);

	// Unequips the specified weapon. Used when OnRep_CurrentWeapon fires.
	void UnEquipWeapon(AWeapon* WeaponToUnEquip);

	void UnEquipCurrentWeapon();

	//add weapon to equipment
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeapon* AddWeaponToEquipment(TSubclassOf<AWeapon> WeaponClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddWeaponToEquipment(TSubclassOf<AWeapon> WeaponClass);

	bool DoesWeaponExistInInventory(AWeapon* InWeapon);

	//EQUIPMENT STUFFS
public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	TArray<AEquipment*> GetEquipmentArray() const;

protected:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipEquipment(AEquipment* NewEquipment);

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerEquipEquipment(AEquipment* NewEquipment);

	//add equipment to equipment list
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AEquipment* AddEquipmentToEquipmentList(TSubclassOf<AEquipment> EquipmentClass);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddEquipmentToEquipmentList(TSubclassOf<AEquipment> EquipmentClass);

	bool DoesEquipmentExistInInventory(AEquipment* InEquipment);

	//ITEM USING / INVENTORY
public:
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;
};
