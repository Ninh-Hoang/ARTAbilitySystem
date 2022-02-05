// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTGameplayAbility.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Blueprint/ARTTargetType.h"
#include "ARTCharacter/ARTSurvivor.h"
#include "GameplayTagContainer.h"
#include "Ability/ARTAbilitySystemGlobals.h"
#include <ARTCharacter/ARTPlayerController.h>
#include <Weapon/Weapon.h>

#include "Ability/ARTGlobalTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UARTGameplayAbility::UARTGameplayAbility()
{
	// Default to Instance Per Actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	bActivateAbilityOnGranted = false;
	bAllowRemoteGrantingActivation = false;
	bActivateOnInput = true;
	bSourceObjectMustEqualCurrentWeaponToActivate = false;
	bCannotActivateWhileInteracting = true;

	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");

	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& (Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass()) || Func->GetOuter()->IsA(
				UDynamicClass::StaticClass()));
	};
	{
		static FName FuncName = FName(TEXT("K2_ScoreAbilityUtility"));
		UFunction* ScoreUtilityFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintScoreUtility = ImplementedInBlueprint(ScoreUtilityFunction);
	}
	
	{
		static FName FuncName = FName(TEXT("K2_GetTargetScore"));
		UFunction* TargetScoreFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetTargetScore = ImplementedInBlueprint(TargetScoreFunction);
	}
	
	{
		static FName FuncName = FName(TEXT("K2_GetOrderTargetData"));
		UFunction* OrderTargetDataFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetOrderTargetData = ImplementedInBlueprint(OrderTargetDataFunction);
	}
	
	{
		static FName FuncName = FName(TEXT("K2_GetRange"));
		UFunction* AbilityRangeFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetRange = ImplementedInBlueprint(AbilityRangeFunction);
	}

	//order system
	AbilityProcessPolicy = EAbilityProcessPolicy::INSTANT;

	GroupExecutionType = EARTOrderGroupExecutionType::MOST_SUITABLE_UNIT;

	AcquisitionRadiusOverride = 0.0f;
	bIsAcquisitionRadiusOverridden = false;

	bHumanPlayerAutoAbility = false;
	bHumanPlayerAutoAutoAbilityInitialState = false;
	bAIPlayerAutoAbility = false;
}


void UARTGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();

	if (bActivateAbilityOnGranted)
	{
		bool ActivatedAbility = AbilitySystemComponent->TryActivateAbility(
			Spec.Handle, bAllowRemoteGrantingActivation);
	}

	//for charged ability
	if (AbilityCharge > 1)
	{
		CurrentCharges = AbilityCharge;
	}

	const FGameplayTagContainer* CDTags = GetCooldownTags();
	if (CDTags)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CDTags->GetByIndex(0), EGameplayTagEventType::AnyCountChange).
		                        AddUObject(this, &UARTGameplayAbility::OnCooldownTagEventCallback);
	}

	//re-check ability trigger when given, this is needed if the ability is given via a GameplayEffect and need to recheck trigger conditions
	const TArray<FAbilityTriggerData>& AbilityTriggerData = AbilityTriggers;

	for (const FAbilityTriggerData& TriggerData : AbilityTriggerData)
	{
		FGameplayTag EventTag = TriggerData.TriggerTag;

		if (TriggerData.TriggerSource != EGameplayAbilityTriggerSource::GameplayEvent)
		{
			if (AbilitySystemComponent->GetTagCount(TriggerData.TriggerTag))
			{
				bool ActivatedAbility = AbilitySystemComponent->TryActivateAbility(
					Spec.Handle, bAllowRemoteGrantingActivation);
			}
		}
	}
}

void UARTGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* const ASC = GetAbilitySystemComponentFromActorInfo();

	//TODO: Should we register on ActivatedAbility or granted?
	//for listen to ASC tag and cancel itself if match AbilityCancelTag
	TArray<FGameplayTag> CancelTagArray;
	AbilityCancelTag.GetGameplayTagArray(CancelTagArray);

	for (FGameplayTag Tag : CancelTagArray)
	{
		FOnGameplayEffectTagCountChanged& Delegate =
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved);

		FDelegateHandle DelegateHandle = Delegate.AddUObject(this, &UARTGameplayAbility::OnCancelTagEventCallback);
		RegisteredCancelTagEventHandles.Add(Tag, DelegateHandle);
	}
}

void UARTGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UAbilitySystemComponent* const ASC = GetAbilitySystemComponentFromActorInfo();
	//for listen to ASC tag and cancel itself if match AbilityCancelTag
	TArray<FGameplayTag> CancelTagArray;
	AbilityCancelTag.GetGameplayTagArray(CancelTagArray);

	for (TPair<FGameplayTag, FDelegateHandle> Pair : RegisteredCancelTagEventHandles)
	{
		FOnGameplayEffectTagCountChanged& Delegate =
			ASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved);

		Delegate.Remove(Pair.Value);
	}
	RegisteredCancelTagEventHandles.Empty();
}

FARTGameplayEffectContainerSpec UARTGameplayAbility::MakeEffectContainerSpecFromContainer(
	const FARTGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// First figure out our actor info
	FARTGameplayEffectContainerSpec ReturnSpec;
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AARTCharacterBase* AvatarCharacter = Cast<AARTCharacterBase>(AvatarActor);
	UARTAbilitySystemComponent* OwningASC = UARTAbilitySystemComponent::GetAbilitySystemComponentFromActor(OwningActor);

	if (OwningASC)
	{
		// If we have a target type, run the targeting logic. This is optional, targets can be added later
		if (Container.TargetType.Get())
		{
			TArray<FHitResult> HitResults;
			TArray<AActor*> TargetActors;
			TArray<FGameplayAbilityTargetDataHandle> TargetData;
			const UARTTargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
			TargetTypeCDO->GetTargets(AvatarCharacter, AvatarActor, EventData, TargetData, HitResults, TargetActors);
			ReturnSpec.AddTargets(TargetData, HitResults, TargetActors);
		}

		// Build GameplayEffectSpecs for each applied effect
		for (const FGameplayEffectInitData& EffectData : Container.TargetGameplayEffect)
		{
			float Level = GetAbilityLevel();

			//master level override has more priority than GameplayEffectInitData
			if (EffectData.LevelOverride != INDEX_NONE) Level = EffectData.LevelOverride;
			if (OverrideGameplayLevel != INDEX_NONE) Level = OverrideGameplayLevel;

			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectData.GameplayEffectClass, OverrideGameplayLevel);
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			
			if (!Spec)
			{
				ABILITY_LOG(Warning, TEXT("UARTGameplayAbility::MakeEffectContainerSpecFromContainer called with invalid SpecHandle"));
				continue;
			}
			
			for(const FGameplayEffectTagMagData& TagData : EffectData.TagMagnitudeData)
			{
				Spec->SetSetByCallerMagnitude(TagData.DataTag, TagData.Magnitude);
			}
			ReturnSpec.TargetGameplayEffectSpecs.Add(SpecHandle);
		}
		
		for (const FGameplayEffectInitData& EffectData : Container.SourceGameplayEffect)
		{
			float Level = GetAbilityLevel();

			//master level override has more priority than GameplayEffectInitData
			if (EffectData.LevelOverride != INDEX_NONE) Level = EffectData.LevelOverride;
			if (OverrideGameplayLevel != INDEX_NONE) Level = OverrideGameplayLevel;

			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectData.GameplayEffectClass, OverrideGameplayLevel);
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			
			if (!Spec)
			{
				ABILITY_LOG(Warning, TEXT("UARTGameplayAbility::MakeEffectContainerSpecFromContainer called with invalid SpecHandle"));
				continue;
			}
			
			for(const FGameplayEffectTagMagData& TagData : EffectData.TagMagnitudeData)
			{
				Spec->SetSetByCallerMagnitude(TagData.DataTag, TagData.Magnitude);
			}
			ReturnSpec.SourceGameplayEffectSpecs.Add(SpecHandle);
		}
	}
	return ReturnSpec;
}

FARTGameplayEffectContainerSpec UARTGameplayAbility::MakeEffectContainerSpec(
	FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	FARTGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	if (FoundContainer)
	{
		return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
	}
	return FARTGameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UARTGameplayAbility::ApplyEffectContainerSpec(
	const FARTGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// Iterate list of effect specs and apply them to their target data
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
	}
	return AllEffects;
}

UObject* UARTGameplayAbility::K2_GetSourceObject(FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo& ActorInfo) const
{
	return GetSourceObject(Handle, &ActorInfo);
}

bool UARTGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
                                                     bool EndAbilityImmediately)
{
	UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (ASC)
	{
		return ASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
	}

	return false;
}

void UARTGameplayAbility::ExternalEndAbility()
{
	check(CurrentActorInfo);

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FString UARTGameplayAbility::GetCurrentPredictionKeyStatus()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (
		ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool UARTGameplayAbility::IsPredictionKeyValidForMorePrediction() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

float UARTGameplayAbility::ScoreAbilityUtility()
{
	if (bHasBlueprintScoreUtility)
	{
		return K2_ScoreAbilityUtility();
	}
	return 0.0f;
}

bool UARTGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayTagContainer* SourceTag,
                                             const FGameplayTagContainer* TargetTag,
                                             OUT FGameplayTagContainer* OptionalRelevantTag) const
{
	if (bCannotActivateWhileInteracting)
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (ASC->GetTagCount(InteractingTag) > ASC->GetTagCount(InteractingRemovalTag))
		{
			return false;
		}
	}

	if (bSourceObjectMustEqualCurrentWeaponToActivate)
	{
		AARTSurvivor* Hero = Cast<AARTSurvivor>(ActorInfo->AvatarActor);

		if (Hero && Hero->GetCurrentWeapon() && static_cast<UObject*>(Hero->GetCurrentWeapon()) == GetSourceObject(
			Handle, ActorInfo))
		{
			return Super::CanActivateAbility(Handle, ActorInfo, SourceTag, TargetTag, OptionalRelevantTag);
		}
		return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTag, TargetTag, OptionalRelevantTag);
}

bool UARTGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    OUT FGameplayTagContainer* OptionalRelevantTag) const
{
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTag) && ARTCheckCost(Handle, *ActorInfo);
}

void UARTGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle =
			MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
		
		float Duration = CooldownDuration.Value;

		//read curve if possible
		if(CooldownDuration.CurveTag.IsValid())
		{
			if(!AbilityData.IsValid())
			{
				AbilityData.LoadSynchronous();
			}
			UARTCurve* Curve = AbilityData.Get();
			if(Curve)
			{
				Duration = Curve->GetCurveValueByTag(CooldownDuration.CurveTag, GetAbilityLevel());
			}
		}
		
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown")),
		                                               Duration);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

bool UARTGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const
{
	if (AbilityCharge > 1)
	{
		const FGameplayTagContainer* CDTags = GetCooldownTags();

		if (CDTags)
		{
			if (CurrentCharges == 0)
			{
				const FGameplayTag& FailCDTag = UAbilitySystemGlobals::Get().ActivateFailCooldownTag;

				if (OptionalRelevantTags && FailCDTag.IsValid())
				{
					OptionalRelevantTags->AddTag(FailCDTag);
				}
				return false;
			}
		}
		return true;
	}
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}

const FGameplayTagContainer* UARTGameplayAbility::GetCooldownTags() const
{
	return &CooldownTags;
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(CooldownTags);
	return MutableTags;
}

void UARTGameplayAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (CurrentCharges > 0)
	{
		CurrentCharges -= 1;
	}
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
}

void UARTGameplayAbility::OnCooldownTagEventCallback(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (AbilityCharge > 1)
	{
		const FGameplayTagContainer* CDTags = GetCooldownTags();
		UAbilitySystemComponent* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CDTags);
		CurrentCharges = AbilityCharge - AbilitySystemComponent->GetAggregatedStackCount(Query);
	}
}

void UARTGameplayAbility::OnCancelTagEventCallback(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

bool UARTGameplayAbility::ARTCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo& ActorInfo) const
{
	return true;
}

void UARTGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	ARTApplyCost(Handle, *ActorInfo, ActivationInfo);
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

int32 UARTGameplayAbility::GetCurrentCharge()
{
	return CurrentCharges;
}

void UARTGameplayAbility::SetHUDReticle(TSubclassOf<UARTHUDReticle> ReticleClass)
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(CurrentActorInfo->PlayerController);
	if (PC)
	{
		PC->SetHUDReticle(ReticleClass);
	}
}

void UARTGameplayAbility::ResetHUDReticle()
{
	AARTPlayerController* PC = Cast<AARTPlayerController>(CurrentActorInfo->PlayerController);
	if (PC)
	{
		AARTSurvivor* Survivor = Cast<AARTSurvivor>(CurrentActorInfo->AvatarActor);
		if (Survivor && Survivor->GetCurrentWeapon())
		{
			PC->SetHUDReticle(Survivor->GetCurrentWeapon()->GetPrimaryHUDReticleClass());
		}
		else
		{
			PC->SetHUDReticle(nullptr);
		}
	}
}

void UARTGameplayAbility::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (IsPredictingClient())
	{
		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(ASC);

		FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

		FGameplayTag ApplicationTag; // Fixme: where would this be useful?
		CurrentActorInfo->AbilitySystemComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
			CurrentActivationInfo.GetActivationPredictionKey(), TargetData, ApplicationTag, ASC->ScopedPredictionKey);
	}
}

bool UARTGameplayAbility::IsInputPressed() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputPressed;
}

UAnimMontage* UARTGameplayAbility::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
	FAbilityMeshMontage AbilityMeshMontage;
	if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
	{
		return AbilityMeshMontage.Montage;
	}

	return nullptr;
}

void UARTGameplayAbility::SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage)
{
	ensure(IsInstantiated());

	FAbilityMeshMontage AbilityMeshMontage;
	if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
	{
		AbilityMeshMontage.Montage = InCurrentMontage;
	}
	else
	{
		CurrentAbilityMeshMontages.Add(FAbilityMeshMontage(InMesh, InCurrentMontage));
	}
}

TArray<FActiveGameplayEffectHandle> UARTGameplayAbility::ApplyEffectContainer(
	FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel /*= -1*/)
{
	FARTGameplayEffectContainerSpec Spec = MakeEffectContainerSpec(ContainerTag, EventData, OverrideGameplayLevel);
	return ApplyEffectContainerSpec(Spec);
}

void UARTGameplayAbility::BP_ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec,
                                                                  FGameplayAbilitySpec& AbilitySpec) const
{
	FGameplayTagContainer& CapturedSourceTags = Spec.CapturedSourceTags.GetSpecTags();

	CapturedSourceTags.AppendTags(AbilityTags);

	// Allow the source object of the ability to propagate tags along as well
	CapturedSourceTags.AppendTags(AbilitySpec.DynamicAbilityTags);

	const IGameplayTagAssetInterface* SourceObjAsTagInterface = Cast<IGameplayTagAssetInterface>(
		AbilitySpec.SourceObject);
	if (SourceObjAsTagInterface)
	{
		FGameplayTagContainer SourceObjTags;
		SourceObjAsTagInterface->GetOwnedGameplayTags(SourceObjTags);

		CapturedSourceTags.AppendTags(SourceObjTags);
	}

	// Copy SetByCallerMagnitudes 
	Spec.MergeSetByCallerMagnitudes(AbilitySpec.SetByCallerTagMagnitudes);
}

bool UARTGameplayAbility::FindAbilityMeshMontage(USkeletalMeshComponent* InMesh,
                                                  FAbilityMeshMontage& InAbilityMeshMontage)
{
	for (FAbilityMeshMontage& MeshMontage : CurrentAbilityMeshMontages)
	{
		if (MeshMontage.Mesh == InMesh)
		{
			InAbilityMeshMontage = MeshMontage;
			return true;
		}
	}

	return false;
}

void UARTGameplayAbility::MontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
	check(CurrentActorInfo);

	UARTAbilitySystemComponent* const AbilitySystemComponent = Cast<UARTAbilitySystemComponent>(
		GetAbilitySystemComponentFromActorInfo_Checked());
	if (AbilitySystemComponent->IsAnimatingAbilityForAnyMesh(this))
	{
		AbilitySystemComponent->CurrentMontageJumpToSectionForMesh(InMesh, SectionName);
	}
}

void UARTGameplayAbility::MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName,
                                                           FName ToSectionName)
{
	check(CurrentActorInfo);

	UARTAbilitySystemComponent* const AbilitySystemComponent = Cast<UARTAbilitySystemComponent>(
		GetAbilitySystemComponentFromActorInfo_Checked());
	if (AbilitySystemComponent->IsAnimatingAbilityForAnyMesh(this))
	{
		AbilitySystemComponent->CurrentMontageSetNextSectionNameForMesh(InMesh, FromSectionName, ToSectionName);
	}
}

void UARTGameplayAbility::MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	check(CurrentActorInfo);

	UARTAbilitySystemComponent* const AbilitySystemComponent = Cast<UARTAbilitySystemComponent>(
		CurrentActorInfo->AbilitySystemComponent.Get());
	if (AbilitySystemComponent != nullptr)
	{
		// We should only stop the current montage if we are the animating ability
		if (AbilitySystemComponent->IsAnimatingAbilityForAnyMesh(this))
		{
			AbilitySystemComponent->CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
		}
	}
}

void UARTGameplayAbility::MontageStopForAllMeshes(float OverrideBlendOutTime)
{
	check(CurrentActorInfo);

	UARTAbilitySystemComponent* const AbilitySystemComponent = Cast<UARTAbilitySystemComponent>(
		CurrentActorInfo->AbilitySystemComponent.Get());
	if (AbilitySystemComponent != nullptr)
	{
		if (AbilitySystemComponent->IsAnimatingAbilityForAnyMesh(this))
		{
			AbilitySystemComponent->StopAllCurrentMontages(OverrideBlendOutTime);
		}
	}
}

// ----------------------------------------------------------------------------------------------------------------
//	ARTAvatarActorInfo Getter
// ----------------------------------------------------------------------------------------------------------------

/*const FARTGameplayAbilityActorInfo* UARTGameplayAbility::GetARTActorInfo(const FGameplayAbilityActorInfo* InInfo) const
{
	return static_cast<const FARTGameplayAbilityActorInfo*>(InInfo);
}

FARTGameplayAbilityActorInfo UARTGameplayAbility::BP_GetARTActorInfo()
{
	return *GetARTActorInfo(CurrentActorInfo);
}

UAnimInstance* UARTGameplayAbility::GetAnimInstance() const
{
	return GetActorInfo().GetAnimInstance();
}

AWeapon* UARTGameplayAbility::BP_GetWeapon() const
{
	const FARTGameplayAbilityActorInfo* KaosActorInfo = GetARTActorInfo(CurrentActorInfo);
	return KaosActorInfo ? KaosActorInfo->GetWeapon() : nullptr;
}*/

// ----------------------------------------------------------------------------------------------------------------
// Order functions
// ----------------------------------------------------------------------------------------------------------------

EARTTargetType UARTGameplayAbility::GetTargetType() const
{
	return TargetType;
}

EARTOrderGroupExecutionType UARTGameplayAbility::GetGroupExecutionType() const
{
	return GroupExecutionType;
}

FGameplayTag UARTGameplayAbility::GetEventTriggerTag() const
{
	for (const FAbilityTriggerData& AbilityTrigger : AbilityTriggers)
	{
		if (AbilityTrigger.TriggerSource == EGameplayAbilityTriggerSource::GameplayEvent)
		{
			return AbilityTrigger.TriggerTag;
		}
	}

	return FGameplayTag();
}

const TArray<FAbilityTriggerData>& UARTGameplayAbility::GetAbilityTriggerData() const
{
	return AbilityTriggers;
}

UTexture2D* UARTGameplayAbility::GetIcon() const
{
	return Icon;
}

FText UARTGameplayAbility::GetName() const
{
	return Name;
}

FText UARTGameplayAbility::GetDescription(const AActor* Actor) const
{
	FText FormattedDescription;
	FormatDescription(Description, Actor, FormattedDescription);
	return FormattedDescription;
}

FARTOrderPreviewData UARTGameplayAbility::GetAbilityPreviewData() const
{
	return AbilityPreviewData;
}

void UARTGameplayAbility::FormatDescription_Implementation(const FText& InDescription, const AActor* Actor,
                                                           FText& OutDescription) const

{
	OutDescription = InDescription;
}

bool UARTGameplayAbility::GetAcquisitionRadiusOverride(float& OutAcquisitionRadius) const
{
	OutAcquisitionRadius = AcquisitionRadiusOverride;
	return bIsAcquisitionRadiusOverridden;
}

int32 UARTGameplayAbility::GetAutoOrderPriority() const
{
	return AutoOrderPriority;
}

bool UARTGameplayAbility::IsHumanPlayerAutoAbility() const
{
	return bHumanPlayerAutoAbility;
}

bool UARTGameplayAbility::GetHumanPlayerAutoAutoAbilityInitialState() const
{
	return bHumanPlayerAutoAutoAbilityInitialState;
}

bool UARTGameplayAbility::IsAIPlayerAutoAbility() const
{
	return bAIPlayerAutoAbility;
}

bool UARTGameplayAbility::IsTargetScoreOverriden() const
{
	return bIsTargetScoreOverridden;
}

bool UARTGameplayAbility::AreAbilityTasksActive() const
{
	return ActiveTasks.Num() > 0;
}

bool UARTGameplayAbility::ShouldActivateAbility(ENetRole Role) const
{
	// This is currently only used by CanActivateAbility to block clients from activating abilities themselves.
	// This in turn is also prevented by NetExecutionPolicy in our case.
	// However, CanActivateAbility is also used by clients in order to check the respective ability before issuing the
	// order.
	return true;
}

void UARTGameplayAbility::OnGameplayTaskActivated(UGameplayTask& Task)
{
	Super::OnGameplayTaskActivated(Task);
}

void UARTGameplayAbility::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	Super::OnGameplayTaskDeactivated(Task);
}

void UARTGameplayAbility::OnAbilityLevelChanged_Implementation(int32 NewLevel)
{
}

float UARTGameplayAbility::GetTargetScore(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData,
                                          int32 Index) const
{
	if(bHasBlueprintGetTargetScore)
	{
		return K2_GetTargetScore(*ActorInfo, Handle, TargetData, Index);
	}
	return 0.0f;
}

bool UARTGameplayAbility::GetOrderTargetData(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, OUT FARTOrderTargetData* OrderTargetData) const
{
	if(bHasBlueprintGetOrderTargetData)
	{
		return K2_GetOrderTargetData(*ActorInfo, Handle, *OrderTargetData);
	}
	return false;
}	

float UARTGameplayAbility::GetRange(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const
{
	if(bHasBlueprintGetRange)
	{
		return K2_GetRange(*ActorInfo, Handle);
	}
	return AbilityBaseRange.GetValue();
}

EAbilityProcessPolicy UARTGameplayAbility::GetAbilityProcessPolicy() const
{
	return AbilityProcessPolicy;
}

FGameplayTagContainer UARTGameplayAbility::GetActivationRequiredTags() const
{
	return ActivationRequiredTags;
}

void UARTGameplayAbility::GetOrderTagRequirements(FARTOrderTagRequirements& OutTagRequirements) const
{
	OutTagRequirements.SourceRequiredTags.AppendTags(SourceRequiredTags);
	OutTagRequirements.SourceBlockedTags.AppendTags(SourceBlockedTags);
	OutTagRequirements.TargetRequiredTags.AppendTags(TargetRequiredTags);
	OutTagRequirements.TargetBlockedTags.AppendTags(TargetBlockedTags);
}

void UARTGameplayAbility::GetSourceTagRequirements(FGameplayTagContainer& OutRequiredTags,
                                                   FGameplayTagContainer& OutBlockedTags) const
{
	OutRequiredTags.AppendTags(SourceRequiredTags);
	OutBlockedTags.AppendTags(SourceBlockedTags);
}

void UARTGameplayAbility::GetTargetTagRequirements(FGameplayTagContainer& OutRequiredTags,
                                                   FGameplayTagContainer& OutBlockedTags) const
{
	OutRequiredTags.AppendTags(TargetRequiredTags);
	OutBlockedTags.AppendTags(TargetBlockedTags);
}

FVector UARTGameplayAbility::GetBlackboardOrderLocation()
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsVector(FName("Order_Location"));
}

AActor* UARTGameplayAbility::GetBlackboardOrderTarget()
{
	return Cast<AActor>(UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsObject(FName("Order_Target")));
}

FVector UARTGameplayAbility::GetBlackboardOrderHomeLocation()
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsVector(FName("Order_HomeLocation"));
}

float UARTGameplayAbility::GetBlackboardOrderRange()
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsFloat(FName("Order_Range"));
}

bool UARTGameplayAbility::DoesSatisfyTargetTagRequirement(AActor* TargetActor)
{
	FGameplayTagContainer TargetTags = FGameplayTagContainer::EmptyContainer;

	if (!TargetActor)
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystem = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (AbilitySystem == nullptr)
	{
		return false;
	}

	AbilitySystem->GetOwnedGameplayTags(TargetTags);

	FGameplayTagContainer RelationshipTags;

	if (GetAvatarActorFromActorInfo() == TargetActor)
	{
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Friendly);
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Self);
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Visible);
	}
	
	const IGenericTeamAgentInterface* SourceCharacter = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	const ETeamAttitude::Type TeamAttitude = SourceCharacter->GetTeamAttitudeTowards(*TargetActor);

	switch (TeamAttitude)
	{
	case ETeamAttitude::Friendly:
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Friendly);
		break;
	case ETeamAttitude::Neutral:
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Neutral);
		break;
	case ETeamAttitude::Hostile:
		RelationshipTags.AddTag(FARTGlobalTags::Get().Behaviour_Hostile);
		break;
	default:
		break;
	}
	
	TargetTags.AppendTags(RelationshipTags);

	if (TargetRequiredTags.Num() || TargetBlockedTags.Num())
	{
		if (TargetTags.HasAny(TargetBlockedTags))
		{
			return false;
		}

		if (!TargetTags .HasAll(TargetRequiredTags))
		{
			return false;
		}
	}

	return true;
}

float UARTGameplayAbility::GetFloatDataFromCurve(const FGameplayTag CurveTag) const
{
	if(!AbilityData.IsValid())
	{
		AbilityData.LoadSynchronous();
	}
	UARTCurve* Curve = AbilityData.Get();
	if(Curve)
	{
		return Curve->GetCurveValueByTag(CooldownDuration.CurveTag, GetAbilityLevel());
	}

	return 0.f;
}
