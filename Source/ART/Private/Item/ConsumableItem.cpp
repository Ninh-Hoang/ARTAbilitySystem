// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ConsumableItem.h"

#include "ARTAssetManager.h"
#include "Ability/ARTGameplayAbility.h"
#include "ARTCharacter/ARTSurvivor.h"
#include "Ability/ARTAbilitySystemComponent.h"

#define LOCTEXT_NAMESPACE "ConsumableItem"

UConsumableItem::UConsumableItem()
{
	ItemType = UARTAssetManager::ConsumableItemType;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}

void UConsumableItem::Use(AARTCharacterBase* Character)
{
	UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(Character->GetAbilitySystemComponent());
	//UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	//UE_LOG(LogTemp, Warning, TEXT("Used"));
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Pawn's ASC is null."), *FString(__FUNCTION__));
		return;
	}

	for (auto& AbilityData : GrantedAbilities)
	{
		const TSubclassOf<UGameplayAbility> AbilityClass = AbilityData.AbilityClass.IsValid() ? AbilityData.AbilityClass.Get() : AbilityData.AbilityClass.LoadSynchronous();
		if (!AbilityClass)
		{
			UE_LOG(LogTemp, Error, TEXT("%s AbilityData is empty."), *FString(__FUNCTION__));
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();
 
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityData.Level);
		AbilitySpec.SourceObject = this;;
        
		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (auto& EffectData : GrantedEffects)
	{
		const TSubclassOf<UGameplayEffect> EffectClass = EffectData.GameplayEffectClass.IsValid() ? EffectData.GameplayEffectClass.Get() :EffectData.GameplayEffectClass.LoadSynchronous();
		if (!EffectClass)
		{
			UE_LOG(LogTemp, Error, TEXT("%s EffectData is empty."), *FString(__FUNCTION__));
			continue;
		}

		FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(EffectClass, EffectData.Level,
		                                                            EffectContext);

		if (NewHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
	}
}

#undef LOCTEXT_NAMESPACE
