// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectUIData.h"
#include "ARTGameplayEffectUIData.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTGameplayEffectUIData : public UGameplayEffectUIData
{
	GENERATED_UCLASS_BODY()

	/** Icon of this Effect. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data)
	UTexture2D* Icon;

	/** Name of this effect. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data)
	FText EffectName;

	/** Description of this effect. Can be shown in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Data, meta = (MultiLine = "true"))
	FText Description;
};
