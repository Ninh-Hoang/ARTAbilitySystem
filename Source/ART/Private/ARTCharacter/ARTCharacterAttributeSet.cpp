// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTCharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include <GameplayEffectExtension.h>



#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/FARTAggregatorEvaluateMetaDataLibrary.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTPlayerController.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

UARTCharacterAttributeSet::UARTCharacterAttributeSet()
{
	/*Health = 100.f;
	MaxHealth = 200.f;
	Stamina = 50.f;
	MaxStamina = 100.f;
	MoveSpeed = 200.f;
	MaxHealth = 200.f;*/

	// Cache tags
	StunTag = FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun"));
	//PartABrokenEventTag= FGameplayTag::RequestGameplayTag(FName("Event.Part.Break.PartA"));
	//PartBBrokenEventTag= FGameplayTag::RequestGameplayTag(FName("Event.Part.Break.PartB"));
}

void UARTCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (Attribute == GetMaxShieldAttribute())
		// GetMaxShieldAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Shield, MaxShield, NewValue, GetShieldAttribute());
	}
	else if (Attribute == GetMaxHealthAttribute())
		// GetMaxHealthAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxEnergyAttribute())
	{
		AdjustAttributeForMaxChange(Energy, MaxEnergy, NewValue, GetEnergyAttribute());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 5000);
	}
	else if (Attribute == GetRotateRateAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 0, 1000);
	}
}

void UARTCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

	//get target actor which should be owner
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	AARTCharacterBase* TargetCharacter = nullptr;

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<AARTCharacterBase>(TargetActor);
	}

	//get the Source Actor
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	AARTCharacterBase* SourceCharacter = nullptr;

	if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
	{
		SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
		SourceController = Source->AbilityActorInfo->PlayerController.Get();
		if (SourceController == nullptr && SourceActor == nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

		//use the controller to find the source pawn
		if (SourceController)
		{
			SourceCharacter = Cast<AARTCharacterBase>(SourceController->GetPawn());
		}
		else
		{
			SourceCharacter = Cast<AARTCharacterBase>(SourceActor);
		}

		if (Context.GetEffectCauser())
		{
			SourceActor = Context.GetEffectCauser();
		}
	}

	//damage
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0.0f)
		{
			bool WasAlive = true;

			if (TargetCharacter)
			{
				WasAlive = TargetCharacter->IsAlive();
			}

			if (!TargetCharacter->IsAlive())
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s() %s is NOT alive when receiving damage"), *FString(__FUNCTION__), *TargetCharacter->GetName());
			}

			const FHitResult* Hit = Data.EffectSpec.GetContext().GetHitResult();
			
			FinalDamageDealing(LocalDamageDone, Hit);

			if (TargetCharacter && WasAlive)
			{
				// This is the log statement for damage received. Turned off for live games.
				//UE_LOG(LogTemp, Log, TEXT("%s() %s Damage Received: %f"), *FString(__FUNCTION__), *GetOwningActor()->GetName(), LocalDamageDone);

				// Show damage number for the Source player unless it was self damage
				if (SourceActor != TargetActor)
				{
					AARTPlayerController* PC = Cast<AARTPlayerController>(SourceController);
					if (PC)
					{
						FGameplayTagContainer DamageNumberTags;

						PC->ShowDamageNumber(LocalDamageDone, TargetCharacter, DamageNumberTags);
					}
				}
			}

			if (!TargetCharacter->IsAlive())
			{
				// TargetCharacter was alive before this damage and now is not alive, give reward if have to Source.
				// Don't give reward to self.
				if (SourceController != TargetController)
				{
					// Create a dynamic instant Gameplay Effect to give the reward
					/*UGameplayEffect* GEBounty = NewObject<UGameplayEffect
					>(GetTransientPackage(), FName(TEXT("Bounty")));
					GEBounty->DurationPolicy = EGameplayEffectDurationType::Instant;

					int32 Idx = GEBounty->Modifiers.Num();
					GEBounty->Modifiers.SetNum(Idx + 2);

					FGameplayModifierInfo& InfoXP = GEBounty->Modifiers[Idx];
					InfoXP.ModifierMagnitude = FScalableFloat(GetXPBounty());
					InfoXP.ModifierOp = EGameplayModOp::Additive;
					InfoXP.Attribute = GetXPAttribute();

					FGameplayModifierInfo& InfoGold = GEBounty->Modifiers[Idx + 1];
					InfoGold.ModifierMagnitude = FScalableFloat(GetEnBounty());
					InfoGold.ModifierOp = EGameplayModOp::Additive;
					InfoGold.Attribute = GetEnAttribute();

					Source->ApplyGameplayEffectToSelf(GEBounty, 1.0f, Source->MakeEffectContext());*/
				}
			}
		}
	}

		//healing
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		//store a local copy of amount of healing done, clear the healing attribute
		const float LocalHealingDone = GetHealing();
		SetHealing(0.0f);

		if (LocalHealingDone > 0.0f)
		{
			bool WasAlive = true;

			if (TargetCharacter)
			{
				WasAlive = TargetCharacter->IsAlive();
			}

			if (!TargetCharacter->IsAlive())
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s() %s is NOT alive when receiving healing"), *FString(__FUNCTION__), *TargetCharacter->GetName());
			}

			//Apply health change and then clamp it
			const float NewHealth = GetHealth() + LocalHealingDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			if (TargetCharacter && WasAlive)
			{
				// This is the log statement for healing received. Turned off for live games.
				//UE_LOG(LogTemp, Log, TEXT("%s() %s Healing Received: %f"), *FString(__FUNCTION__), *GetOwningActor()->GetName(), LocalHealingDone);

				// Show healing number for the Source player unless it was self damage
				if (SourceActor != TargetActor)
				{
					AARTPlayerController* PC = Cast<AARTPlayerController>(SourceController);
					if (PC)
					{
						FGameplayTagContainer HealingNumberTags;

						//PC->ShowDamageNumber(LocalDamageDone, TargetCharacter, DamageNumberTags)
					}
				}
			}
		}
	}

		// shield
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		// Handle shield changes.
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes.
		// Health loss should go through Damage.
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		// Handle other Energy changes.
		SetEnergy(FMath::Clamp(GetEnergy(), 0.0f, GetMaxEnergy()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Handle stamina changes.
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
} 

void UARTCharacterAttributeSet::FinalDamageDealing(float LocalDamage, const FHitResult* Hit)
{
	//apply damage to shield first if exists
	const float OldShield = GetShield();
	float DamageAfterShield = LocalDamage - OldShield;
	if (OldShield > 0)
	{
		float NewShield = OldShield - LocalDamage;
		SetShield(FMath::Clamp<float>(NewShield, 0.0f, GetMaxShield()));
	}
	
	if (DamageAfterShield > 0)
	{
		if(Hit && Hit->GetComponent() && Hit->Component->ComponentHasTag(FName("PartA")))
		{
			// Apply the health change and then clamp it
			const float NewHealth = GetPartHealthA() - DamageAfterShield;
			SetPartHealthA(FMath::Clamp(NewHealth, 0.0f, 999999.f));

			if(GetPartHealthA() <= 0.0f)
			{
				// Create a dynamic infinite Gameplay Effect 
				UGameplayEffect* GEPartAProken = NewObject<UGameplayEffect
				>(GetTransientPackage(), FName(TEXT("PartABroken")));
				GEPartAProken->DurationPolicy = EGameplayEffectDurationType::Infinite;
				
				GEPartAProken->InheritableOwnedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("State.LegBot.LeftGunBroken")));
				
				GetOwningAbilitySystemComponent()->ApplyGameplayEffectToSelf(GEPartAProken, 1.0f, GetOwningAbilitySystemComponent()->MakeEffectContext());
				FGameplayEventData Payload;
				
				//Add info to playload here
				GetOwningAbilitySystemComponent()->HandleGameplayEvent(PartABrokenEventTag, &Payload);
			}
		}
		else
		{
			// Apply the health change and then clamp it
			const float NewHealth = GetHealth() - DamageAfterShield;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
}

void UARTCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, CritRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ReactMas, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PhysBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PhysRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, HealBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, IncomingHealBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, VoidBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, VoidRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, HeatBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, HeatRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ColdBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ColdRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ElecBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ElecRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, EarthBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, EarthRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, AirBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, AirRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, LifeBonus, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, LifeRes, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, ShieldRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthA, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthB, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthC, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthD, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthE, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, PartHealthF, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, Energy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, EnergyRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, StaminaRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, RotateRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, XPMod, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UARTCharacterAttributeSet, EnMod, COND_None, REPNOTIFY_Always);
}

void UARTCharacterAttributeSet::OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute,
                                                             FAggregator* NewAggregator) const
{
	Super::OnAttributeAggregatorCreated(Attribute, NewAggregator);

	if (!NewAggregator)
	{
		return;
	}

	if (Attribute == GetMoveSpeedAttribute())
	{
		NewAggregator->EvaluationMetaData = &
			FARTAggregatorEvaluateMetaDataLibrary::MostNegativeMod_MostPositiveModPerClass;
	}
}


