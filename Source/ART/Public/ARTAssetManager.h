// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ARTAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UARTAssetManager () {}
	virtual void StartInitialLoading() override;
	
	static const FPrimaryAssetType AbilitySetType;
	static const FPrimaryAssetType ConsumableItemType;
	static const FPrimaryAssetType SkillItemType;
	
	FORCEINLINE static UARTAssetManager& Get();
};
