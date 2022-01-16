// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Order/ARTUseAbilityOrder.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"

#include "Ability/ARTGlobalTags.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

UARTUseAbilityOrder::UARTUseAbilityOrder()
{
}

bool UARTUseAbilityOrder::CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                                       FARTOrderErrorTags* OutErrorTags /*= nullptr*/) const
{
	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	const UGameplayAbility* Ability = GetAbility(AbilitySystem, OrderTags);

	if (Ability != nullptr)
	{
		const TArray<FGameplayAbilitySpec>& AbilitySpecs = AbilitySystem->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
		{
			if (Spec.Ability == Ability)
			{
				// Check if ability has been learned yet.
				/*if (Spec.Level <= 0)
				{
					return false;
				}*/

				FGameplayTagContainer FailureTags;

				// Don't pass any source and target tags to can activate ability. These tags has already been checked in
				// 'UARTOrderHelper'. Only the activation required and activation blocked tags are checked here.
				if (!Ability->CanActivateAbility(Spec.Handle, AbilitySystem->AbilityActorInfo.Get(), nullptr, nullptr,
				                                 &FailureTags))
				{
					if (OutErrorTags != nullptr)
					{
						OutErrorTags->ErrorTags = FailureTags;
					}

					return false;
				}

				// Not the nicest place to check this but it avoids adding this tag to every ability.
				/*if (AbilitySystem->HasMatchingGameplayTag(UARTGlobalTags::Status_Changing_Constructing()))
				{
					return false;
				}*/

				return true;
			}
		}
	}

	return false;
}

EARTTargetType UARTUseAbilityOrder::GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                  int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return EARTTargetType::NONE;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetTargetType();
	}

	return EARTTargetType::NONE;
}

void UARTUseAbilityOrder::IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                     const FGameplayTagContainer& OrderTags, int32 Index,
                                     FARTOrderCallback Callback, const FVector& HomeLocation)
{
	if (OrderedActor == nullptr)
	{
		UE_LOG(LogOrder, Error, TEXT("Ordered actor is invalid."));
		Callback.Broadcast(EARTOrderResult::FAILED);
		return;
	}


	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UGameplayAbility* Ability = GetAbility(AbilitySystem, OrderTags);

	if (GetOrderProcessPolicy(OrderedActor, OrderTags, Index) == EARTOrderProcessPolicy::INSTANT)
	{
		if (AbilitySystem->TryActivateAbilityByClass(Ability->GetClass()))
		{
			Callback.Broadcast(EARTOrderResult::SUCCEEDED);
		}
		else
		{
			Callback.Broadcast(EARTOrderResult::FAILED);
		}
		/*FGameplayEventData EventData;
		UARTAbilitySystemHelper::CreateGameplayEventData(OrderedActor, TargetData, Ability->GetClass(), EventData);

		int32 TriggeredAbilities = UARTAbilitySystemHelper::SendGameplayEvent(OrderedActor, EventData);
		if (TriggeredAbilities > 0)
		{
			Callback.Broadcast(EARTOrderResult::SUCCEEDED);
		}

		else
		{
			Callback.Broadcast(EARTOrderResult::FAILED);
		}*/
	}

	else
	{
		Super::IssueOrder(OrderedActor, TargetData, OrderTags, Index, Callback, HomeLocation);
	}
}

UTexture2D* UARTUseAbilityOrder::GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                               int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

UTexture2D* UARTUseAbilityOrder::GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	return GetIcon(OrderedActor, OrderTags, Index);
}

FText UARTUseAbilityOrder::GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                   int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'OrderedActor' was 'nullptr'."));
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'Index' was invalid."));
	}

	return Ability->GetName();
}

FText UARTUseAbilityOrder::GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                          int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FText::FromString(
			TEXT("UARTUseAbilityOrder::GetDescription: Error: Parameter 'OrderedActor' was 'nullptr'."));
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FText::FromString(TEXT("UARTUseAbilityOrder::GetName: Error: Parameter 'Index' was invalid."));
	}

	return Ability->GetDescription(OrderedActor);
}

FARTOrderPreviewData UARTUseAbilityOrder::GetOrderPreviewData(const AActor* OrderedActor,
                                                              const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return FARTOrderPreviewData();
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return FARTOrderPreviewData();
	}

	return Ability->GetAbilityPreviewData();
}

EARTOrderProcessPolicy UARTUseAbilityOrder::GetOrderProcessPolicy(const AActor* OrderedActor,
                                                                  const FGameplayTagContainer& OrderTags,
                                                                  int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return Super::GetOrderProcessPolicy(OrderedActor, OrderTags, Index);
	}

	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return Super::GetOrderProcessPolicy(OrderedActor, OrderTags, Index);
	}

	EARTAbilityProcessPolicy AbilityProcessPolicy = Ability->GetAbilityProcessPolicy();

	// If this ability does not have a target type or location and is instant we can execute it here directly without
	// altering the AI behavior.
	if ((Ability->GetTargetType() == EARTTargetType::NONE || Ability->GetTargetType() == EARTTargetType::PASSIVE) &&
		AbilityProcessPolicy == EARTAbilityProcessPolicy::INSTANT)
	{
		return EARTOrderProcessPolicy::INSTANT;
	}

	if (AbilityProcessPolicy == EARTAbilityProcessPolicy::CAN_NOT_BE_CANCELED)
	{
		return EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED;
	}

	if (AbilityProcessPolicy == EARTAbilityProcessPolicy::CAN_BE_CANCELED)
	{
		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	if (AbilityProcessPolicy == EARTAbilityProcessPolicy::CAN_BE_CANCELED_WHEN_NO_GAMEPLAY_TASK_IS_RUNNING)
	{
		if (Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// It should not be possible to run ability tasks using a non instanced ability.
			return EARTOrderProcessPolicy::CAN_BE_CANCELED;
		}

		FGameplayAbilitySpec AbilitySpec;

		const TArray<FGameplayAbilitySpec>& AbilitySpecs = AbilitySystem->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
		{
			if (Spec.Ability == Ability)
			{
				AbilitySpec = Spec;
				break;
			}
		}

		if (AbilitySpec.Ability == nullptr)
		{
			return EARTOrderProcessPolicy::CAN_BE_CANCELED;
		}

		// Iterate through every active instance to check if any of them has an active ability task.
		TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : AbilityInstances)
		{
			UARTGameplayAbility* ARTAbilityInstance = Cast<UARTGameplayAbility>(AbilityInstance);

			if (AbilityInstance != nullptr && ARTAbilityInstance->AreAbilityTasksActive())
			{
				return EARTOrderProcessPolicy::CAN_NOT_BE_CANCELED;
			}
		}

		return EARTOrderProcessPolicy::CAN_BE_CANCELED;
	}

	return EARTOrderProcessPolicy::CAN_BE_CANCELED;
}

EARTOrderGroupExecutionType UARTUseAbilityOrder::GetGroupExecutionType(const AActor* OrderedActor,
                                                                       const FGameplayTagContainer& OrderTags,
                                                                       int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return EARTOrderGroupExecutionType::ALL;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetGroupExecutionType();
	}

	return EARTOrderGroupExecutionType::ALL;
}

bool UARTUseAbilityOrder::IsHumanPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->IsHumanPlayerAutoAbility();
	}

	return false;
}

bool UARTUseAbilityOrder::GetHumanPlayerAutoOrderInitialState(const AActor* OrderedActor,
                                                              const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetHumanPlayerAutoAutoAbilityInitialState();
	}

	return false;
}

bool UARTUseAbilityOrder::IsAIPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                              int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->IsAIPlayerAutoAbility();
	}

	return false;
}

bool UARTUseAbilityOrder::GetAcquisitionRadiusOverride(const AActor* OrderedActor,
                                                       const FGameplayTagContainer& OrderTags, int32 Index,
                                                       float& OutAcquisitionRadius) const
{
	if (OrderedActor == nullptr)
	{
		return false;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetAcquisitionRadiusOverride(OutAcquisitionRadius);
	}

	return false;
}

float UARTUseAbilityOrder::GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
                                          const FGameplayTagContainer& OrderTags, int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability == nullptr || !Ability->IsTargetScoreOverriden())
	{
		return Super::GetTargetScore(OrderedActor, TargetData, OrderTags, Index);
	}

	return Ability->GetTargetScore(TargetData, Index);
}

void UARTUseAbilityOrder::GetTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                             int32 Index,
                                             FARTOrderTagRequirements& OutTagRequirements) const
{
	if (OrderedActor == nullptr)
	{
		return;
	}
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));

	if (Ability != nullptr)
	{
		return Ability->GetOrderTagRequirements(OutTagRequirements);
	}
}

float UARTUseAbilityOrder::GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                            int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return 0.0f;
	}

	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	return AbilitySystem->GetAbilityRange(OrderTags);
}

void UARTUseAbilityOrder::InitializePreviewActor(AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
                                                 const FARTOrderTargetData& TargetData,
                                                 const FGameplayTagContainer& OrderTags, int32 Index)
{
	UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UGameplayAbility* Ability = GetAbility(AbilitySystem, OrderTags);
	PreviewActor->SetInstigatorAbility(Cast<UARTGameplayAbility>(Ability));
}

UGameplayAbility* UARTUseAbilityOrder::GetAbility(const UARTAbilitySystemComponent* AbilitySystem, int32 Index) const
{
	for (const FGameplayAbilitySpec& Spec : AbilitySystem->GetActivatableAbilities())
	{
		if (Spec.InputID == Index)
		{
			return Spec.Ability;
		}
	}

	return nullptr;
}

UGameplayAbility* UARTUseAbilityOrder::GetAbility(const UARTAbilitySystemComponent* AbilitySystem,
                                                  FGameplayTagContainer OrderTags) const
{
	TArray<FGameplayAbilitySpec*> SpecArray;
	AbilitySystem->GetActivatableGameplayAbilitySpecsByAllMatchingTags(OrderTags, SpecArray, false);
	if (SpecArray.Num() > 0)
	{
		return SpecArray[0]->Ability;
	}

	return nullptr;
}

UTexture2D* UARTUseAbilityOrder::GetIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                         int32 Index) const
{
	if (OrderedActor == nullptr)
	{
		return nullptr;
	}

	const UARTAbilitySystemComponent* AbilitySystem = OrderedActor->FindComponentByClass<UARTAbilitySystemComponent>();
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(GetAbility(AbilitySystem, OrderTags));
	if (Ability == nullptr)
	{
		return nullptr;
	}

	return Ability->GetIcon();
}
