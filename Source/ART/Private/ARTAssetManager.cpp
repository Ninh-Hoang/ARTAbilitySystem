// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "Ability/ARTGlobalTags.h"

const FPrimaryAssetType	UARTAssetManager::AbilitySetType = TEXT("AbilitySet");
const FPrimaryAssetType	UARTAssetManager::ConsumableItemType = TEXT("ConsumableItem");
const FPrimaryAssetType	UARTAssetManager::SkillItemType = TEXT("SkillItem");

UARTAssetManager& UARTAssetManager::Get()
{
	UARTAssetManager* This = Cast<UARTAssetManager>(GEngine->AssetManager);

	if (This)
	{
		return *This;
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManager in DefaultEngine.ini, must be ARTAssetManager!"));
		return *NewObject<UARTAssetManager>(); // never calls this
	}
}

void UARTAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	//TODO: still have no idea why we do this here
	FARTGlobalTags::InitializeNativeTags();
	UAbilitySystemGlobals::Get().InitGlobalData();
}
