// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "GATA_AoeTrace.generated.h"

/**
 * 
 */
UCLASS()
class ART_API AGATA_AoeTrace : public AGameplayAbilityTargetActor_GroundTrace
{
	GENERATED_BODY()

	AGATA_AoeTrace();

	virtual void Tick(float DeltaTime) override;
};
