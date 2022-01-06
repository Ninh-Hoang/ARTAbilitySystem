// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/RangeWeapon.h"
#include "AbilitySystemInterface.h"
#include "TracerRangeWeapon.generated.h"

/**
 * 
 */
class AGATA_LineTrace;
class UARTAbilitySystemComponent;

USTRUCT()
struct FHitScanTrace
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

UCLASS(Blueprintable, BlueprintType)
class ART_API ATracerRangeWeapon : public ARangeWeapon
{
	GENERATED_BODY()
public:
	ATracerRangeWeapon();

	~ATracerRangeWeapon();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	virtual void BeginPlay() override;

	virtual void ThreatTrace() override;
};
