// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "Ability/ARTGameplayAbility.h"
#include "GameplayCueManager.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include <Blueprint/ARTCurve.h>
#include <Ability/ARTGameplayEffect.h>
#include <Ability/ARTGameplayEffectTypes.h>
#include <Blueprint/ARTBlueprintFunctionLibrary.h>
#include <AbilitySystemBlueprintLibrary.h>

UARTAbilitySystemComponent::UARTAbilitySystemComponent()
{
}

void UARTAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	OnGameplayEffectAppliedDelegateToTarget.AddUObject(
		this, &UARTAbilitySystemComponent::OnGameplayEffectAppliedToTargetCallback);
	OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		this, &UARTAbilitySystemComponent::OnActiveGameplayEffectAppliedToSelfCallback);
}

//TODO MAYBE THIS CAN BE CHEAPER
void UARTAbilitySystemComponent::OnGameplayEffectAppliedToTargetCallback(
	UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	const UARTGameplayEffect* Effect = Cast<UARTGameplayEffect>(SpecApplied.Def);
	if (!Effect || Effect->GameplayEvents.Num() < 1 || Effect->DurationPolicy != EGameplayEffectDurationType::Instant)
	{
		return;
	}

	for (FGameplayEffectEvent Event : Effect->GameplayEvents)
	{
		FGameplayEventData Data;
		FGameplayTag GameplayEventTag;

		AActor* EventInstigator = nullptr;
		AActor* EventTarget = nullptr;

		Event.AttempAssignGameplayEventDataActors(GetAvatarActor(), Target->GetAvatarActor(), EventInstigator,
		                                          EventTarget);

		Data.Instigator = EventInstigator;
		Data.Target = EventTarget;

		Event.AttemptCalculateMagnitude(SpecApplied, Data.EventMagnitude, false);

		const FGameplayTagContainer* InstigatorTags = SpecApplied.CapturedSourceTags.GetAggregatedTags();
		const FGameplayTagContainer* TargetTags = SpecApplied.CapturedTargetTags.GetAggregatedTags();

		Event.AttemptReturnGameplayEventTags(InstigatorTags, TargetTags, GameplayEventTag, Data.InstigatorTags,
		                                     Data.TargetTags);

		if (const FHitResult* Hit = SpecApplied.GetEffectContext().Get()->GetHitResult())
		{
			Data.TargetData = UARTBlueprintFunctionLibrary::AbilityTargetDataFromHitResult(*Hit);
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EventTarget, GameplayEventTag, Data);
	}
}

void UARTAbilitySystemComponent::OnActiveGameplayEffectAppliedToSelfCallback(
	UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	const UARTGameplayEffect* Effect = Cast<UARTGameplayEffect>(SpecApplied.Def);
	if (!Effect || Effect->GameplayEvents.Num() < 1
		|| Effect->DurationPolicy == EGameplayEffectDurationType::Instant)
	{
		return;
	}

	for (FGameplayEffectEvent Event : Effect->GameplayEvents)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"),
		       *SpecApplied.GetEffectContext().GetInstigatorAbilitySystemComponent()->GetAvatarActor()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Target->GetAvatarActor()->GetName());

		FGameplayEventData Data;
		FGameplayTag GameplayEventTag;

		AActor* SourceAvatarActor = SpecApplied.GetEffectContext().GetInstigatorAbilitySystemComponent()->
		                                        GetAvatarActor();
		AActor* TargetAvatarActor = GetAvatarActor();

		AActor* EventInstigator = nullptr;
		AActor* EventTarget = nullptr;

		Event.AttempAssignGameplayEventDataActors(SourceAvatarActor, GetAvatarActor(), EventInstigator, EventTarget);

		Data.Instigator = EventInstigator;
		Data.Target = EventTarget;

		Event.AttemptCalculateMagnitude(SpecApplied, Data.EventMagnitude, false);

		const FGameplayTagContainer* InstigatorTags = SpecApplied.CapturedSourceTags.GetAggregatedTags();
		const FGameplayTagContainer* TargetTags = SpecApplied.CapturedTargetTags.GetAggregatedTags();

		Event.AttemptReturnGameplayEventTags(InstigatorTags, TargetTags, GameplayEventTag, Data.InstigatorTags,
		                                     Data.TargetTags);

		if (const FHitResult* Hit = SpecApplied.GetEffectContext().Get()->GetHitResult())
		{
			Data.TargetData = UARTBlueprintFunctionLibrary::AbilityTargetDataFromHitResult(*Hit);
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EventTarget, GameplayEventTag, Data);
	}
}

void UARTAbilitySystemComponent::ReceiveDamage(UARTAbilitySystemComponent* SourceASC, float UnmitigatedDamage,
                                               float MitigatedDamage)
{
	ReceivedDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
}

static TAutoConsoleVariable<float> CVarReplayMontageErrorThreshold(
	TEXT("GS.replay.MontageErrorThreshold"),
	0.5f,
	TEXT("Tolerance level for when montage playback position correction occurs in replays")
);

void UARTAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARTAbilitySystemComponent, RepAnimMontageInfoForMeshes);
}

bool UARTAbilitySystemComponent::GetShouldTick() const
{
	for (FGameplayAbilityRepAnimMontageForMesh RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		const bool bHasReplicatedMontageInfoToUpdate = (IsOwnerActorAuthoritative() && RepMontageInfo.RepMontageInfo.
			IsStopped == false);

		if (bHasReplicatedMontageInfoToUpdate)
		{
			return true;
		}
	}

	return Super::GetShouldTick();
}

void UARTAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	if (IsOwnerActorAuthoritative())
	{
		for (FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
		{
			AnimMontage_UpdateReplicatedDataForMesh(MontageInfo.Mesh);
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UARTAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	LocalAnimMontageInfoForMeshes = TArray<FGameplayAbilityLocalAnimMontageForMesh>();
	RepAnimMontageInfoForMeshes = TArray<FGameplayAbilityRepAnimMontageForMesh>();

	if (bPendingMontageRep)
	{
		OnRep_ReplicatedAnimMontageForMesh();
	}
}

void UARTAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
                                                    bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	// If AnimatingAbility ended, clear the pointer
	ClearAnimatingAbilityForAllMeshes(Ability);
}

void UARTAbilitySystemComponent::CancelAbilitiesWithTag(const FGameplayTagContainer WithTags,
                                                        const FGameplayTagContainer WithoutTags,
                                                        UGameplayAbility* Ignore)
{
	CancelAbilities(&WithTags, &WithoutTags, Ignore);
}

UARTAbilitySystemComponent* UARTAbilitySystemComponent::GetAbilitySystemComponentFromActor(
	const AActor* Actor, bool LookForComponent)
{
	return Cast<UARTAbilitySystemComponent>(
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

void UARTAbilitySystemComponent::AbilityLocalInputPressed(int32 InputID)
{
	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}

	// ---------------------------------------------------------

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID)
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle,
					                      Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					UARTGameplayAbility* GA = Cast<UARTGameplayAbility>(Spec.Ability);
					if (GA && GA->bActivateOnInput)
					{
						// Ability is not active, so try to activate it
						TryActivateAbility(Spec.Handle);
					}
				}
			}
		}
	}
}

int32 UARTAbilitySystemComponent::K2_GetTagCount(FGameplayTag TagToCheck) const
{
	return GetTagCount(TagToCheck);
}

FGameplayAbilitySpecHandle UARTAbilitySystemComponent::FindAbilitySpecHandleForClass(
	TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
		if (SpecAbilityClass == AbilityClass)
		{
			if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
			{
				return Spec.Handle;
			}
		}
	}
	return FGameplayAbilitySpecHandle();
}

int32 UARTAbilitySystemComponent::FindAbilityChargeViaCooldownTag(FGameplayTagContainer InCooldownTag)
{
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		const FGameplayTagContainer* CooldownTag = Spec.Ability->GetCooldownTags();
		if (CooldownTag->HasAllExact(InCooldownTag))
		{
			UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(Spec.Ability);
			Ability->GetCurrentCharge();
		}
	}
	return 0;
}

void UARTAbilitySystemComponent::K2_AddLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
	AddLooseGameplayTag(GameplayTag, Count);
}

void UARTAbilitySystemComponent::K2_AddLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
	AddLooseGameplayTags(GameplayTags, Count);
}

void UARTAbilitySystemComponent::K2_RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
	RemoveLooseGameplayTag(GameplayTag, Count);
}

void UARTAbilitySystemComponent::K2_RemoveLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
	RemoveLooseGameplayTags(GameplayTags, Count);
}

bool UARTAbilitySystemComponent::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
                                                            bool EndAbilityImmediately)
{
	bool AbilityActivated = false;
	if (InAbilityHandle.IsValid())
	{
		FScopedServerAbilityRPCBatcher AbilityRPCBatcher(this, InAbilityHandle);

		AbilityActivated = TryActivateAbility(InAbilityHandle, true);

		if (EndAbilityImmediately)
		{
			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(InAbilityHandle);
			if (AbilitySpec)
			{
				UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(AbilitySpec->GetPrimaryInstance());
				Ability->ExternalEndAbility();
			}
		}
		return AbilityActivated;
	}
	return AbilityActivated;
}

void UARTAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                         const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UARTAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                     const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
}

void UARTAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                        const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

FString UARTAbilitySystemComponent::GetCurrentPredictionKeyStatus()
{
	return ScopedPredictionKey.ToString() + " is valid for more prediction: " + (
		ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

FActiveGameplayEffectHandle UARTAbilitySystemComponent::BP_ApplyGameplayEffectToSelfWithPrediction(
	TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle EffectContext)
{
	if (GameplayEffectClass)
	{
		if (!EffectContext.IsValid())
		{
			EffectContext = MakeEffectContext();
		}

		UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();

		if (CanPredict())
		{
			return ApplyGameplayEffectToSelf(GameplayEffect, Level, EffectContext, ScopedPredictionKey);
		}

		return ApplyGameplayEffectToSelf(GameplayEffect, Level, EffectContext);
	}

	return FActiveGameplayEffectHandle();
}

FActiveGameplayEffectHandle UARTAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction(
	TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level,
	FGameplayEffectContextHandle Context)
{
	if (Target == nullptr)
	{
		ABILITY_LOG(
			Log,
			TEXT(
				"UAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction called with null Target. %s. Context: %s"
			), *GetFullName(), *Context.ToString());
		return FActiveGameplayEffectHandle();
	}

	if (GameplayEffectClass == nullptr)
	{
		ABILITY_LOG(
			Error,
			TEXT(
				"UAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction called with null GameplayEffectClass. %s. Context: %s"
			), *GetFullName(), *Context.ToString());
		return FActiveGameplayEffectHandle();
	}

	UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();

	if (CanPredict())
	{
		return ApplyGameplayEffectToTarget(GameplayEffect, Target, Level, Context, ScopedPredictionKey);
	}

	return ApplyGameplayEffectToTarget(GameplayEffect, Target, Level, Context);
}

FActiveGameplayEffectHandle UARTAbilitySystemComponent::BP_ApplyGameplayEffectSpecToSelfWithPrediction(
	const FGameplayEffectSpec& GameplayEffect)
{
	if (CanPredict())
	{
		return ApplyGameplayEffectSpecToSelf(GameplayEffect, ScopedPredictionKey);
	}

	return ApplyGameplayEffectSpecToSelf(GameplayEffect);
}

FActiveGameplayEffectHandle UARTAbilitySystemComponent::BP_ApplyGameplayEffectSpecToTargetWithPrediction(
	const FGameplayEffectSpec& GameplayEffect, UAbilitySystemComponent* Target)
{
	if (Target == nullptr)
	{
		ABILITY_LOG(
			Log,
			TEXT(
				"UAbilitySystemComponent::BP_ApplyGameplayEffectSpecToTargetWithPrediction called with null Target. %s. Context: %s"
			), *GetFullName(), *GameplayEffect.GetEffectContext().ToString());
		return FActiveGameplayEffectHandle();
	}

	if (CanPredict())
	{
		return ApplyGameplayEffectSpecToTarget(GameplayEffect, Target, ScopedPredictionKey);
	}

	return ApplyGameplayEffectSpecToTarget(GameplayEffect, Target);
}

bool UARTAbilitySystemComponent::SetGameplayEffectDurationHandle(FActiveGameplayEffectHandle Handle, float NewDuration)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffect* ActiveGameplayEffect = GetActiveGameplayEffect(Handle);
	if (!ActiveGameplayEffect)
	{
		return false;
	}

	FActiveGameplayEffect* AGE = const_cast<FActiveGameplayEffect*>(ActiveGameplayEffect);
	if (NewDuration > 0)
	{
		AGE->Spec.Duration = NewDuration;
	}
	else
	{
		AGE->Spec.Duration = 0.01f;
	}

	AGE->StartServerWorldTime = ActiveGameplayEffects.GetServerWorldTime();
	AGE->CachedStartServerWorldTime = AGE->StartServerWorldTime;
	AGE->StartWorldTime = ActiveGameplayEffects.GetWorldTime();
	ActiveGameplayEffects.MarkItemDirty(*AGE);
	ActiveGameplayEffects.CheckDuration(Handle);

	AGE->EventSet.OnTimeChanged.Broadcast(AGE->Handle, AGE->StartWorldTime, AGE->GetDuration());
	OnGameplayEffectDurationChange(*AGE);

	return true;
}

bool UARTAbilitySystemComponent::AddGameplayEffectDurationHandle(FActiveGameplayEffectHandle Handle, float AddDuration)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffect* ActiveGameplayEffect = GetActiveGameplayEffect(Handle);
	if (!ActiveGameplayEffect)
	{
		return false;
	}


	FActiveGameplayEffect* AGE = const_cast<FActiveGameplayEffect*>(ActiveGameplayEffect);
	if (AddDuration > 0.f)
	{
		AGE->Spec.Duration += AddDuration;
	}
	if (AddDuration < 0.f)
	{
		if (AGE->Spec.Duration + AddDuration > 0.01f)
		{
			AGE->Spec.Duration += AddDuration;
		}
		else
		{
			AGE->Spec.Duration = 0.01f;
		}
	}

	AGE->StartServerWorldTime = ActiveGameplayEffects.GetServerWorldTime();
	AGE->CachedStartServerWorldTime = AGE->StartServerWorldTime;
	AGE->StartWorldTime = ActiveGameplayEffects.GetWorldTime();
	ActiveGameplayEffects.MarkItemDirty(*AGE);
	ActiveGameplayEffects.CheckDuration(Handle);

	AGE->EventSet.OnTimeChanged.Broadcast(AGE->Handle, AGE->StartWorldTime, AGE->GetDuration());
	OnGameplayEffectDurationChange(*AGE);

	return true;
}

FGameplayEffectSpecHandle UARTAbilitySystemComponent::MakeOutgoingSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass,
                                                                       float Level,
                                                                       FGameplayEffectContextHandle Context) const
{
	FGameplayEffectSpecHandle Spec = Super::MakeOutgoingSpec(GameplayEffectClass, Level, Context);
	if (Spec.IsValid())
	{
		UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
		if (UARTGameplayEffect* ArtGE = Cast<UARTGameplayEffect>(GameplayEffect))
		{
			//TODO: some strange execution flow here, is soft pointer is valid, the hard ref would be too
			if(!ArtGE->Curves.IsValid())
			{
				ArtGE->Curves.LoadSynchronous();
			}
			
			UARTCurve* GECurve = ArtGE->Curves.Get();
			
			if (GECurve)
			{
				//for each curve in ARTCurve asset, take the curve's tag and use it to SetByCallerMagnitude for GE
				FGameplayTagContainer TagList;
				GECurve->GetCurveTagList(TagList);

				for(auto& Tag : TagList)
					Spec.Data->SetSetByCallerMagnitude(Tag, GECurve->GetCurveValueByTag(Tag, Level));
			}
		}
		return Spec;
	}
	return FGameplayEffectSpecHandle(nullptr);
}

//FOR AI OR UI
/* Returns a list of currently active ability instances that match the tags */
void UARTAbilitySystemComponent::GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer,
                                                            TArray<UARTGameplayAbility*>& ActiveAbilities)
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, AbilitiesToActivate, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			ActiveAbilities.Add(Cast<UARTGameplayAbility>(ActiveAbility));
		}
	}
}

void UARTAbilitySystemComponent::GetActiveEffectHandlesByClass(TSubclassOf<UGameplayEffect> SourceGameplayEffect,
                                                               TArray<FActiveGameplayEffectHandle>&
                                                               OutActiveEffectHandles)
{
	OutActiveEffectHandles.Reset();

	if (SourceGameplayEffect)
	{
		FGameplayEffectQuery Query;
		Query.CustomMatchDelegate.BindLambda([&](const FActiveGameplayEffect& CurEffect)
		{
			return CurEffect.Spec.Def && SourceGameplayEffect == CurEffect.Spec.Def->GetClass();
		});

		OutActiveEffectHandles = ActiveGameplayEffects.GetActiveEffects(Query);
	}
}

float UARTAbilitySystemComponent::PlayMontageForMesh(UGameplayAbility* InAnimatingAbility,
                                                     USkeletalMeshComponent* InMesh,
                                                     FGameplayAbilityActivationInfo ActivationInfo,
                                                     UAnimMontage* NewAnimMontage, float InPlayRate,
                                                     FName StartSectionName, bool bReplicateMontage)
{
	UARTGameplayAbility* InAbility = Cast<UARTGameplayAbility>(InAnimatingAbility);

	float Duration = -1.f;

	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
		if (Duration > 0.f)
		{
			FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

			if (AnimMontageInfo.LocalMontageInfo.AnimatingAbility && AnimMontageInfo.LocalMontageInfo.AnimatingAbility
				!= InAnimatingAbility)
			{
				// The ability that was previously animating will have already gotten the 'interrupted' callback.
				// It may be a good idea to make this a global policy and 'cancel' the ability.
				// 
				// For now, we expect it to end itself when this happens.
			}

			if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UAbilitySystemComponent::PlayMontage %s, Role: %s")
				       , *GetNameSafe(NewAnimMontage)
				       , *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole(
				       ))
				);
			}

			AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
			AnimMontageInfo.LocalMontageInfo.AnimatingAbility = InAnimatingAbility;
			AnimMontageInfo.LocalMontageInfo.PlayBit = !AnimMontageInfo.LocalMontageInfo.PlayBit;

			if (InAbility)
			{
				InAbility->SetCurrentMontageForMesh(InMesh, NewAnimMontage);
			}

			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
			}

			// Replicate to non owners
			if (IsOwnerActorAuthoritative())
			{
				if (bReplicateMontage)
				{
					// Those are static parameters, they are only set when the montage is played. They are not changed after that.
					FGameplayAbilityRepAnimMontageForMesh& AbilityRepMontageInfo =
						GetGameplayAbilityRepAnimMontageForMesh(InMesh);
					AbilityRepMontageInfo.RepMontageInfo.AnimMontage = NewAnimMontage;
					AbilityRepMontageInfo.RepMontageInfo.ForcePlayBit = !static_cast<bool>(AbilityRepMontageInfo.
						RepMontageInfo.ForcePlayBit);

					// Update parameters that change during Montage life time.
					AnimMontage_UpdateReplicatedDataForMesh(InMesh);

					// Force net update on our avatar actor
					if (AbilityActorInfo->AvatarActor != nullptr)
					{
						AbilityActorInfo->AvatarActor->ForceNetUpdate();
					}
				}
			}
			else
			{
				// If this prediction key is rejected, we need to end the preview
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					PredictionKey.NewRejectedDelegate().BindUObject(
						this, &UARTAbilitySystemComponent::OnPredictiveMontageRejectedForMesh, InMesh, NewAnimMontage);
				}
			}
		}
	}

	return Duration;
}

float UARTAbilitySystemComponent::PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh,
                                                              UAnimMontage* NewAnimMontage, float InPlayRate,
                                                              FName StartSectionName)
{
	float Duration = -1.f;
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
		if (Duration > 0.f)
		{
			FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
			AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
		}
	}

	return Duration;
}

void UARTAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	UAnimMontage* MontageToStop = AnimMontageInfo.LocalMontageInfo.AnimMontage;
	bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f
			                            ? OverrideBlendOutTime
			                            : MontageToStop->BlendOut.GetBlendTime());

		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
	}
}

void UARTAbilitySystemComponent::StopAllCurrentMontages(float OverrideBlendOutTime)
{
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes
	)
	{
		CurrentMontageStopForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, OverrideBlendOutTime);
	}
}

void UARTAbilitySystemComponent::StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh,
                                                             const UAnimMontage& Montage, float OverrideBlendOutTime)
{
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (&Montage == AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
	}
}

void UARTAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability)
{
	UARTGameplayAbility* GSAbility = Cast<UARTGameplayAbility>(Ability);
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes
	)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == Ability)
		{
			GSAbility->SetCurrentMontageForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, nullptr);
			GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility = nullptr;
		}
	}
}

void UARTAbilitySystemComponent::CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if ((SectionName != NAME_None) && AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		AnimInstance->Montage_JumpToSection(SectionName, AnimMontageInfo.LocalMontageInfo.AnimMontage);
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			ServerCurrentMontageJumpToSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage,
			                                             SectionName);
		}
	}
}

void UARTAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh(
	USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
	{
		// Set Next Section Name. 
		AnimInstance->Montage_SetNextSection(FromSectionName, ToSectionName,
		                                     AnimMontageInfo.LocalMontageInfo.AnimMontage);

		// Update replicated version for Simulated Proxies if we are on the server.
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			float CurrentPosition = AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage);
			ServerCurrentMontageSetNextSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage,
			                                              CurrentPosition, FromSectionName, ToSectionName);
		}
	}
}

void UARTAbilitySystemComponent::CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
	{
		// Set Play Rate
		AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);

		// Update replicated version for Simulated Proxies if we are on the server.
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			ServerCurrentMontageSetPlayRateForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);
		}
	}
}

void UARTAbilitySystemComponent::BP_SetPlayRateForCurrentMontage(USkeletalMeshComponent* InMesh, float InPlayRate)
{
	CurrentMontageSetPlayRateForMesh(InMesh, InPlayRate);
}

bool UARTAbilitySystemComponent::IsAnimatingAbilityForAnyMesh(UGameplayAbility* InAbility) const
{
	for (FGameplayAbilityLocalAnimMontageForMesh GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == InAbility)
		{
			return true;
		}
	}

	return false;
}

UGameplayAbility* UARTAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh()
{
	// Only one ability can be animating for all meshes
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes
	)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility)
		{
			return GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility;
		}
	}

	return nullptr;
}

TArray<UAnimMontage*> UARTAbilitySystemComponent::GetCurrentMontages() const
{
	TArray<UAnimMontage*> Montages;

	for (FGameplayAbilityLocalAnimMontageForMesh GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
	{
		UAnimInstance* AnimInstance = IsValid(GameplayAbilityLocalAnimMontageForMesh.Mesh)
		                              && GameplayAbilityLocalAnimMontageForMesh.Mesh->GetOwner() == AbilityActorInfo->
		                              AvatarActor
			                              ? GameplayAbilityLocalAnimMontageForMesh.Mesh->GetAnimInstance()
			                              : nullptr;

		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage && AnimInstance
			&& AnimInstance->Montage_IsActive(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage))
		{
			Montages.Add(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage);
		}
	}

	return Montages;
}

TArray<UAnimMontage*> UARTAbilitySystemComponent::BP_GetCurrentMontages() const
{
	return GetCurrentMontages();
}

UAnimMontage* UARTAbilitySystemComponent::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance
		&& AnimInstance->Montage_IsActive(AnimMontageInfo.LocalMontageInfo.AnimMontage))
	{
		return AnimMontageInfo.LocalMontageInfo.AnimMontage;
	}

	return nullptr;
}

int32 UARTAbilitySystemComponent::GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		return CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);
	}

	return INDEX_NONE;
}

FName UARTAbilitySystemComponent::GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);

		return CurrentAnimMontage->GetSectionName(CurrentSectionID);
	}

	return NAME_None;
}

float UARTAbilitySystemComponent::GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		int32 CurrentSectionID = GetCurrentMontageSectionIDForMesh(InMesh);
		if (CurrentSectionID != INDEX_NONE)
		{
			TArray<FCompositeSection>& CompositeSections = CurrentAnimMontage->CompositeSections;

			// If we have another section after us, then take delta between both start times.
			if (CurrentSectionID < (CompositeSections.Num() - 1))
			{
				return (CompositeSections[CurrentSectionID + 1].GetTime() - CompositeSections[CurrentSectionID].
					GetTime());
			}
			// Otherwise we are the last section, so take delta with Montage total time.
			return (CurrentAnimMontage->GetPlayLength() - CompositeSections[CurrentSectionID].GetTime());
		}

		// if we have no sections, just return total length of Montage.
		return CurrentAnimMontage->GetPlayLength();
	}

	return 0.f;
}

float UARTAbilitySystemComponent::GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance && AnimInstance->Montage_IsActive(CurrentAnimMontage))
	{
		const float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		return CurrentAnimMontage->GetSectionTimeLeftFromPos(CurrentPosition);
	}

	return -1.f;
}


FGameplayAbilityLocalAnimMontageForMesh& UARTAbilitySystemComponent::GetLocalAnimMontageInfoForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
	{
		if (MontageInfo.Mesh == InMesh)
		{
			return MontageInfo;
		}
	}

	FGameplayAbilityLocalAnimMontageForMesh MontageInfo = FGameplayAbilityLocalAnimMontageForMesh(InMesh);
	LocalAnimMontageInfoForMeshes.Add(MontageInfo);
	return LocalAnimMontageInfoForMeshes.Last();
}

FGameplayAbilityRepAnimMontageForMesh& UARTAbilitySystemComponent::GetGameplayAbilityRepAnimMontageForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FGameplayAbilityRepAnimMontageForMesh& RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		if (RepMontageInfo.Mesh == InMesh)
		{
			return RepMontageInfo;
		}
	}

	FGameplayAbilityRepAnimMontageForMesh RepMontageInfo = FGameplayAbilityRepAnimMontageForMesh(InMesh);
	RepAnimMontageInfoForMeshes.Add(RepMontageInfo);
	return RepAnimMontageInfoForMeshes.Last();
}

void UARTAbilitySystemComponent::OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh,
                                                                    UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && PredictiveMontage)
	{
		// If this montage is still playing: kill it
		if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
		{
			AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
		}
	}
}

void UARTAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh)
{
	check(IsOwnerActorAuthoritative());

	AnimMontage_UpdateReplicatedDataForMesh(GetGameplayAbilityRepAnimMontageForMesh(InMesh));
}

void UARTAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(
	FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
	UAnimInstance* AnimInstance = IsValid(OutRepAnimMontageInfo.Mesh) && OutRepAnimMontageInfo.Mesh->GetOwner()
	                              == AbilityActorInfo->AvatarActor
		                              ? OutRepAnimMontageInfo.Mesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
		OutRepAnimMontageInfo.Mesh);

	if (AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		OutRepAnimMontageInfo.RepMontageInfo.AnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;

		// Compressed Flags
		bool bIsStopped = AnimInstance->Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);

		if (!bIsStopped)
		{
			OutRepAnimMontageInfo.RepMontageInfo.PlayRate = AnimInstance->Montage_GetPlayRate(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
			OutRepAnimMontageInfo.RepMontageInfo.Position = AnimInstance->Montage_GetPosition(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
			OutRepAnimMontageInfo.RepMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
		}

		if (OutRepAnimMontageInfo.RepMontageInfo.IsStopped != bIsStopped)
		{
			// Set this prior to calling UpdateShouldTick, so we start ticking if we are playing a Montage
			OutRepAnimMontageInfo.RepMontageInfo.IsStopped = bIsStopped;

			// When we start or stop an animation, update the clients right away for the Avatar Actor
			if (AbilityActorInfo->AvatarActor != nullptr)
			{
				AbilityActorInfo->AvatarActor->ForceNetUpdate();
			}

			// When this changes, we should update whether or not we should be ticking
			UpdateShouldTick();
		}

		// Replicate NextSectionID to keep it in sync.
		// We actually replicate NextSectionID+1 on a BYTE to put INDEX_NONE in there.
		int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(
			OutRepAnimMontageInfo.RepMontageInfo.Position);
		if (CurrentSectionID != INDEX_NONE)
		{
			int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(
				AnimMontageInfo.LocalMontageInfo.AnimMontage, CurrentSectionID);
			if (NextSectionID >= (256 - 1))
			{
				ABILITY_LOG(
					Error,
					TEXT(
						"AnimMontage_UpdateReplicatedData. NextSectionID = %d.  RepAnimMontageInfo.Position: %.2f, CurrentSectionID: %d. LocalAnimMontageInfo.AnimMontage %s"
					),
					NextSectionID, OutRepAnimMontageInfo.RepMontageInfo.Position, CurrentSectionID,
					*GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage));
				ensure(NextSectionID < (256 - 1));
			}
			OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = static_cast<uint8>(NextSectionID + 1);
		}
		else
		{
			OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = 0;
		}
	}
}

void UARTAbilitySystemComponent::AnimMontage_UpdateForcedPlayFlagsForMesh(
	FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
		OutRepAnimMontageInfo.Mesh);

	OutRepAnimMontageInfo.RepMontageInfo.ForcePlayBit = AnimMontageInfo.LocalMontageInfo.PlayBit;
}

void UARTAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
	for (FGameplayAbilityRepAnimMontageForMesh& NewRepMontageInfoForMesh : RepAnimMontageInfoForMeshes)
	{
		FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
			NewRepMontageInfoForMesh.Mesh);

		UWorld* World = GetWorld();

		if (NewRepMontageInfoForMesh.RepMontageInfo.bSkipPlayRate)
		{
			NewRepMontageInfoForMesh.RepMontageInfo.PlayRate = 1.f;
		}

		const bool bIsPlayingReplay = World && World->IsPlayingReplay();

		const float MONTAGE_REP_POS_ERR_THRESH = bIsPlayingReplay
			                                         ? CVarReplayMontageErrorThreshold.GetValueOnGameThread()
			                                         : 0.1f;

		UAnimInstance* AnimInstance = IsValid(NewRepMontageInfoForMesh.Mesh) && NewRepMontageInfoForMesh.Mesh->
		                              GetOwner()
		                              == AbilityActorInfo->AvatarActor
			                              ? NewRepMontageInfoForMesh.Mesh->GetAnimInstance()
			                              : nullptr;
		if (AnimInstance == nullptr || !IsReadyForReplicatedMontageForMesh())
		{
			// We can't handle this yet
			bPendingMontageRep = true;
			return;
		}
		bPendingMontageRep = false;

		if (!AbilityActorInfo->IsLocallyControlled())
		{
			static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.Montage.Debug"));
			bool DebugMontage = (CVar && CVar->GetValueOnGameThread() == 1);
			if (DebugMontage)
			{
				ABILITY_LOG(Warning, TEXT("\n\nOnRep_ReplicatedAnimMontage, %s"), *GetNameSafe(this));
				ABILITY_LOG(
					Warning,
					TEXT(
						"\tAnimMontage: %s\n\tPlayRate: %f\n\tPosition: %f\n\tBlendTime: %f\n\tNextSectionID: %d\n\tIsStopped: %d\n\tForcePlayBit: %d"
					),
					*GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage),
					NewRepMontageInfoForMesh.RepMontageInfo.PlayRate,
					NewRepMontageInfoForMesh.RepMontageInfo.Position,
					NewRepMontageInfoForMesh.RepMontageInfo.BlendTime,
					NewRepMontageInfoForMesh.RepMontageInfo.NextSectionID,
					NewRepMontageInfoForMesh.RepMontageInfo.IsStopped,
					NewRepMontageInfoForMesh.RepMontageInfo.ForcePlayBit);
				ABILITY_LOG(Warning, TEXT("\tLocalAnimMontageInfo.AnimMontage: %s\n\tPosition: %f"),
				            *GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage),
				            AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage));
			}

			if (NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage)
			{
				// New Montage to play
				const bool ReplicatedPlayBit = static_cast<bool>(NewRepMontageInfoForMesh.RepMontageInfo.ForcePlayBit);
				if ((AnimMontageInfo.LocalMontageInfo.AnimMontage != NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage
				) || (AnimMontageInfo.LocalMontageInfo.PlayBit != ReplicatedPlayBit))
				{
					AnimMontageInfo.LocalMontageInfo.PlayBit = ReplicatedPlayBit;
					PlayMontageSimulatedForMesh(NewRepMontageInfoForMesh.Mesh,
					                            NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage,
					                            NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}

				if (AnimMontageInfo.LocalMontageInfo.AnimMontage == nullptr)
				{
					ABILITY_LOG(
						Warning,
						TEXT("OnRep_ReplicatedAnimMontage: PlayMontageSimulated failed. Name: %s, AnimMontage: %s"),
						*GetNameSafe(this), *GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage));
					return;
				}

				// Play Rate has changed
				if (AnimInstance->Montage_GetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage) !=
					NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
				{
					AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage,
					                                  NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}

				// Compressed Flags
				const bool bIsStopped = AnimInstance->
					Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);
				const bool bReplicatedIsStopped = static_cast<bool>(NewRepMontageInfoForMesh.RepMontageInfo.IsStopped);

				// Process stopping first, so we don't change sections and cause blending to pop.
				if (bReplicatedIsStopped)
				{
					if (!bIsStopped)
					{
						CurrentMontageStopForMesh(NewRepMontageInfoForMesh.Mesh,
						                          NewRepMontageInfoForMesh.RepMontageInfo.BlendTime);
					}
				}
				else if (!NewRepMontageInfoForMesh.RepMontageInfo.SkipPositionCorrection)
				{
					const int32 RepSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
					                                           GetSectionIndexFromPosition(
						                                           NewRepMontageInfoForMesh.RepMontageInfo.Position);
					const int32 RepNextSectionID = static_cast<int32>(NewRepMontageInfoForMesh.RepMontageInfo.
						NextSectionID) - 1;

					// And NextSectionID for the replicated SectionID.
					if (RepSectionID != INDEX_NONE)
					{
						const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(
							AnimMontageInfo.LocalMontageInfo.AnimMontage, RepSectionID);

						// If NextSectionID is different than the replicated one, then set it.
						if (NextSectionID != RepNextSectionID)
						{
							AnimInstance->Montage_SetNextSection(
								AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepSectionID),
								AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepNextSectionID),
								AnimMontageInfo.LocalMontageInfo.AnimMontage);
						}

						// Make sure we haven't received that update too late and the client hasn't already jumped to another section. 
						const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
						                                               GetSectionIndexFromPosition(
							                                               AnimInstance->Montage_GetPosition(
								                                               AnimMontageInfo.LocalMontageInfo.
								                                               AnimMontage));
						if ((CurrentSectionID != RepSectionID) && (CurrentSectionID != RepNextSectionID))
						{
							// Client is in a wrong section, teleport him into the begining of the right section
							const float SectionStartTime = AnimMontageInfo.LocalMontageInfo.AnimMontage->
							                                               GetAnimCompositeSection(RepSectionID).
							                                               GetTime();
							AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage,
							                                  SectionStartTime);
						}
					}

					// Update Position. If error is too great, jump to replicated position.
					const float CurrentPosition = AnimInstance->Montage_GetPosition(
						AnimMontageInfo.LocalMontageInfo.AnimMontage);
					const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
					                                               GetSectionIndexFromPosition(CurrentPosition);
					const float DeltaPosition = NewRepMontageInfoForMesh.RepMontageInfo.Position - CurrentPosition;

					// Only check threshold if we are located in the same section. Different sections require a bit more work as we could be jumping around the timeline.
					// And therefore DeltaPosition is not as trivial to determine.
					if ((CurrentSectionID == RepSectionID) && (FMath::Abs(DeltaPosition) > MONTAGE_REP_POS_ERR_THRESH)
						&& (NewRepMontageInfoForMesh.RepMontageInfo.IsStopped == 0))
					{
						// fast forward to server position and trigger notifies
						if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(
							NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage))
						{
							// Skip triggering notifies if we're going backwards in time, we've already triggered them.
							const float DeltaTime =
								!FMath::IsNearlyZero(NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
									? (DeltaPosition / NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
									: 0.f;
							if (DeltaTime >= 0.f)
							{
								MontageInstance->UpdateWeight(DeltaTime);
								MontageInstance->HandleEvents(CurrentPosition,
								                              NewRepMontageInfoForMesh.RepMontageInfo.Position,
								                              nullptr);
								AnimInstance->TriggerAnimNotifies(DeltaTime);
							}
						}
						AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage,
						                                  NewRepMontageInfoForMesh.RepMontageInfo.Position);
					}
				}
			}
		}
	}
}

bool UARTAbilitySystemComponent::IsReadyForReplicatedMontageForMesh()
{
	/** Children may want to override this for additional checks (e.g, "has skin been applied") */
	return true;
}

void UARTAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName,
	FName NextSectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			// Set NextSectionName
			AnimInstance->Montage_SetNextSection(SectionName, NextSectionName, CurrentAnimMontage);

			// Correct position if we are in an invalid section
			float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
			int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(CurrentPosition);
			FName CurrentSectionName = CurrentAnimMontage->GetSectionName(CurrentSectionID);

			int32 ClientSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(ClientPosition);
			FName ClientCurrentSectionName = CurrentAnimMontage->GetSectionName(ClientSectionID);
			if ((CurrentSectionName != ClientCurrentSectionName) || (CurrentSectionName != SectionName))
			{
				// We are in an invalid section, jump to client's position.
				AnimInstance->Montage_SetPosition(CurrentAnimMontage, ClientPosition);
			}

			// Update replicated version for Simulated Proxies if we are on the server.
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UARTAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Validate(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName,
	FName NextSectionName)
{
	return true;
}

void UARTAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			// Set NextSectionName
			AnimInstance->Montage_JumpToSection(SectionName, CurrentAnimMontage);

			// Update replicated version for Simulated Proxies if we are on the server.
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UARTAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Validate(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName)
{
	return true;
}

void UARTAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			// Set PlayRate
			AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);

			// Update replicated version for Simulated Proxies if we are on the server.
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UARTAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Validate(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate)
{
	return true;
}

/*
* Order System
*/

void UARTAbilitySystemComponent::GetAutoOrders_Implementation(TArray<FARTOrderTypeWithIndex>& OutAutoOrders)
{
	TArray<FGameplayAbilitySpec> ActivatableSpecs = ActivatableAbilities.Items;
	for (int32 Index = 0; Index < ActivatableSpecs.Num(); ++Index)
	{
		TSubclassOf<UGameplayAbility> AbilityType = ActivatableSpecs[Index].Ability->GetClass();

		if (AbilityType == nullptr)
		{
			continue;
		}

		UARTGameplayAbility* Ability = AbilityType->GetDefaultObject<UARTGameplayAbility>();
		if (Ability == nullptr)
		{
			continue;
		}

		if (Ability->GetTargetType() != EARTTargetType::PASSIVE)
		{
			int32 OrderIndex = OutAutoOrders.Num();
			for(int32 i = 0; i < OutAutoOrders.Num(); i ++)
			{
				if(Ability->GetAutoOrderPriority() <= OutAutoOrders[i].Index)
				{
					OrderIndex = i;
					break;
				}
			}
			OutAutoOrders.Insert(FARTOrderTypeWithIndex(UseAbilityOrder, Ability->GetAutoOrderPriority(), Ability->AbilityTags), OrderIndex);
		}
	}
}

FOnAutoOrderUpdate* UARTAbilitySystemComponent::GetAutoOrderAddDelegate()
{
	return &OnAutoOrderAdded;
}

FOnAutoOrderUpdate* UARTAbilitySystemComponent::GetAutoOrderRemoveDelegate()
{
	return &OnAutoOrderRemove;
}

void UARTAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(AbilitySpec.Ability);
	if (Ability->GetTargetType() != EARTTargetType::PASSIVE)
	{
		OnAutoOrderAdded.Broadcast((FARTOrderTypeWithIndex(UseAbilityOrder, Ability->GetAutoOrderPriority(), Ability->AbilityTags)));
	}
}

void UARTAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
	UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(AbilitySpec.Ability);
	if (Ability->GetTargetType() != EARTTargetType::PASSIVE)
	{
		OnAutoOrderRemove.Broadcast((FARTOrderTypeWithIndex(UseAbilityOrder, Ability->GetAutoOrderPriority(), Ability->AbilityTags)));
	}
}
