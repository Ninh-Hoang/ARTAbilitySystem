#include "ARTCharacter/AttributeSet/ARTAttributeSet_Damage.h"
#include "GameplayEffectExtension.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTPlayerController.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Health.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_Shield.h"
#include "ARTCharacter/AttributeSet/ARTAttributeSet_TraumaHealth.h"

UARTAttributeSet_Damage::UARTAttributeSet_Damage()
{
}

void UARTAttributeSet_Damage::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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
}

void UARTAttributeSet_Damage::FinalDamageDealing(float LocalDamage, const FHitResult* Hit)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	//apply damage to shield first if exists
	const float OldShield = ASC->GetNumericAttribute(UARTAttributeSet_Shield::GetShieldAttribute());
	const float DamageAfterShield = LocalDamage - OldShield;

	if (OldShield > 0.f)
	{
		ASC->ApplyModToAttributeUnsafe(UARTAttributeSet_Shield::GetShieldAttribute(), EGameplayModOp::Additive, -LocalDamage);
	}

	const float OldHealth = ASC->GetNumericAttribute(UARTAttributeSet_Health::GetHealthAttribute());
	if (DamageAfterShield > 0.f && OldHealth > 0.f)
	{
		ASC->ApplyModToAttributeUnsafe(UARTAttributeSet_Health::GetHealthAttribute(), EGameplayModOp::Additive, -DamageAfterShield);

		if(ASC->HasAttributeSetForAttribute(UARTAttributeSet_TraumaHealth::GetTraumaHealthAttribute()))
		{
			ASC->ApplyModToAttributeUnsafe(UARTAttributeSet_TraumaHealth::GetTraumaHealthAttribute(), EGameplayModOp::Additive, DamageAfterShield*0.1f );
		}
	}
}
