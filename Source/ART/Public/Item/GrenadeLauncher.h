// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "GrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class ART_API AGrenadeLauncher : public AWeaponActor
{
	GENERATED_BODY()


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> ProjectileClass;
};
