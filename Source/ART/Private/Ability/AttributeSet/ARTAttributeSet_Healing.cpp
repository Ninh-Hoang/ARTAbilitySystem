#include "Ability/AttributeSet/ARTAttributeSet_Healing.h"
#include "GameplayEffectExtension.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTPlayerController.h"
#include "Ability/AttributeSet/ARTAttributeSet_Health.h"

UARTAttributeSet_Healing::UARTAttributeSet_Healing()
{
}

void UARTAttributeSet_Healing::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
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

	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
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
			const float NewHealth = GetOwningAbilitySystemComponent()->GetNumericAttribute(UARTAttributeSet_Health::GetHealthAttribute()) + LocalHealingDone;
			const float MaxHealth = GetOwningAbilitySystemComponent()->GetNumericAttribute(UARTAttributeSet_Health::GetMaxHealthAttribute());

			const float ClampedNewHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

			GetOwningAbilitySystemComponent()->SetNumericAttributeBase(UARTAttributeSet_Health::GetHealthAttribute(), ClampedNewHealth);
			
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
}
