// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipment.h"
#include "WeaponActor.generated.h"

class UDamageType;
class UParticleSystem;
class UMeshComponent;

//contain weapon of LineTrace
USTRUCT()
struct FHitScanTrac
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

	UPROPERTY()
	uint8 StructIncreasement;
};

UCLASS()
class ART_API AWeaponActor : public AEquipment
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponActor();

protected:

	float LastFireTime;

	FTimerHandle TimerHandle_TimeBetweenShot;

	//RPM
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	//derived from RateOfFire
	float TimeBetweenShot;

	UPROPERTY(EditDefaultsOnly)
	float ThreatValue;

	UPROPERTY(EditDefaultsOnly)
	float FirstShotDelay;

	UPROPERTY(EditDefaultsOnly)
	float FireRotationSpeed;

	float OwnerRotationSpeed;

	UPROPERTY(EditDefaultsOnly)
	float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UMeshComponent* MeshComponent;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void Initialize(UMeshComponent* MeshComponentToSet);
};
