// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include <AbilitySystemInterface.h>
#include "ART/ART.h"
#include "Equipment.generated.h"

class AARTSurvivor;
class USkeletalMeshComponent;

class UARTGameplayAbility;
class UAbilitySystemComponent;
class UARTAbilitySystemComponent;

UENUM()
enum class EEquipmentSlot: uint8
{
	ES_WeaponRest UMETA(DisplayName = "WeaponRestSocket"),
	ES_Pelvis UMETA(DisplayName = "PelvisSocket"),
	ES_Shield UMETA(DisplayName = "ShieldSocket"),
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class ART_API AEquipment : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEquipment();

	~AEquipment();

	// Implement IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Equipment")
	FGameplayTag EquipmentTag;

	//TARGET ACTORS
	UPROPERTY()
	class AGATA_LineTrace* LineTraceTargetActor;

	UPROPERTY()
	class AGATA_AoeTrace* AoeTargetActor;

	UPROPERTY()
	class AGATA_MeleeWeaponTrace* MeleeTargetActor;

	UPROPERTY()
	class AGATA_SphereTrace* SphereTargetActor;

	// Getter for LineTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "ART|Targeting")
	class AGATA_LineTrace* GetLineTraceTargetActor();

	UFUNCTION(BlueprintCallable, Category = "ART|Targeting")
	class AGATA_SphereTrace* GetSphereTraceTargetActor();

	// Getter for MeleeTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "ART|Targeting")
	class AGATA_MeleeWeaponTrace* GetMeleeTraceTargetActor();

	// Getter for MeleeTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "ART|Targeting")
	class AGATA_AoeTrace* GetAoeTraceTargetActor();

	UPROPERTY(VisibleDefaultsOnly, Category = "ART|Equipment")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ART|Equipment")
	class AARTCharacterBase* OwningCharacter;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Equipment")
	FText DefaultStatusText;

	UPROPERTY()
	UARTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "ART|Equipment")
	TArray<TSubclassOf<UARTGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "ART|Equipment")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Animation")
	UAnimMontage* EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "ART|Equipment")
	EEquipmentSlot EquipmentSlot;

public:
	//EQUIPMENT STUFFS
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "ART|Equipment")
	FText StatusText;

	virtual void Equip(class AARTSurvivor* CharacterToEquip);

	virtual void UnEquip();

	// Resets things like fire mode to default
	UFUNCTION(BlueprintCallable, Category = "ART|Equipment")
	virtual void ResetEquipment();

	UFUNCTION(BlueprintCallable, Category = "ART|Equipment")
	FText GetDefaultStatusText() const;

	//ABILITIES STUFFS
	void SetOwningCharacter(AARTCharacterBase* InOwningCharacter);

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	virtual int32 GetAbilityLevel(EARTAbilityInputID AbilityID);

	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "ART|Equipment")
	USkeletalMeshComponent* GetEquipmentMesh();

	UFUNCTION(BlueprintCallable, Category = "ART|Animation")
	UAnimMontage* GetEquipMontage() const;
};
