// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayTagContainer.h"
#include "ARTDamageTextWidgetComponent.generated.h"

/**
 * For the floating Damage Numbers when a Character receives damage.
 */
UCLASS()
class ART_API UARTDamageTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage, const FGameplayTagContainer& Tags);
};
