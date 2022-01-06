// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ConsumableItem.h"
#include "Ability/ARTGameplayAbility.h"
#include "ARTCharacter/ARTSurvivor.h"
#include <Ability/ARTAbilitySystemComponent.h>

#define LOCTEXT_NAMESPACE "FoodItem"

UConsumableItem::UConsumableItem()
{
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}

void UConsumableItem::Use(AARTSurvivor* Character)
{
	UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(Character->GetAbilitySystemComponent());
	//UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	//UE_LOG(LogTemp, Warning, TEXT("Used"));
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Pawn's ASC is null."), *FString(__FUNCTION__));
		return;
	}

	for (TSubclassOf<UARTGameplayAbility> AbilityClass : AbilityClasses)
	{
		if (!AbilityClass)
		{
			continue;
		}
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1,
		                                                        static_cast<int32>(AbilityClass.GetDefaultObject()->
			                                                        AbilityInputID), this);
		ASC->GiveAbilityAndActivateOnce(AbilitySpec);
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> EffectClass : EffectClasses)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(EffectClass, Character->GetCharacterLevel(),
		                                                            EffectContext);

		if (NewHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
	}
}

#undef LOCTEXT_NAMESPACE
