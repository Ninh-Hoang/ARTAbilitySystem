// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_PlayMontageMeshWaitForEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ART/ART.h"
#include "GameFramework/Character.h"

UAT_PlayMontageMeshWaitForEvent::UAT_PlayMontageMeshWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
	bTickingTask = false;
}

UARTAbilitySystemComponent* UAT_PlayMontageMeshWaitForEvent::GetTargetASC()
{
	return Cast<UARTAbilitySystemComponent>(AbilitySystemComponent);
}

void UAT_PlayMontageMeshWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent->ClearAnimatingAbility(Ability);

			//reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() ==
					EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UAT_PlayMontageMeshWaitForEvent::OnAbilityCancelled()
{
	// TODO: Merge this fix back to engine, it was calling the wrong callback
	if (StopPlayingMontage(OverrideBlendOutTimeForCancelAbility))
	{
		// Let the BP handle the interrupt as well
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UAT_PlayMontageMeshWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void UAT_PlayMontageMeshWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}

UAT_PlayMontageMeshWaitForEvent* UAT_PlayMontageMeshWaitForEvent::PlayMontageForMeshAndWaitForEvent(
	UGameplayAbility* OwningAbility, FName TaskInstanceName,
	USkeletalMeshComponent* InMesh, UAnimMontage* MontageToPlay,
	FGameplayTagContainer EventTags, float Rate /*= 1.f*/,
	FName StartSection /*= NAME_None*/, bool bStopWhenAbilityEnds /*= true*/,
	float AnimRootMotionTranslationScale /*= 1.f*/, bool bReplicateMontage /*= true*/,
	float OverrideBlendOutTimeForCancelAbility /*= -1.f*/,
	float OverrideBlendOutTimeForStopWhenEndAbility /*= -1.f*/)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UAT_PlayMontageMeshWaitForEvent* MyObj = NewAbilityTask<UAT_PlayMontageMeshWaitForEvent>(
		OwningAbility, TaskInstanceName);
	MyObj->Mesh = InMesh;
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->bReplicateMontage = bReplicateMontage;
	MyObj->OverrideBlendOutTimeForCancelAbility = OverrideBlendOutTimeForCancelAbility;
	MyObj->OverrideBlendOutTimeForStopWhenEndAbility = OverrideBlendOutTimeForStopWhenEndAbility;

	return MyObj;
}

void UAT_PlayMontageMeshWaitForEvent::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	if (Mesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s invalid Mesh"), *FString(__FUNCTION__));
		return;
	}

	bool bPlayedMontage = false;
	UARTAbilitySystemComponent* ARTAbilitySystemComponent = GetTargetASC();

	if (AbilitySystemComponent)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// Bind to event callback
			EventHandle = ARTAbilitySystemComponent->AddGameplayEventTagContainerDelegate(
				EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
					this, &UAT_PlayMontageMeshWaitForEvent::OnGameplayEvent));

			if (ARTAbilitySystemComponent->PlayMontageForMesh(Ability, Mesh, Ability->GetCurrentActivationInfo(),
			                                                  MontageToPlay, Rate, StartSection,
			                                                  bReplicateMontage) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(
					this, &UAT_PlayMontageMeshWaitForEvent::OnAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &UAT_PlayMontageMeshWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAT_PlayMontageMeshWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() ==
						EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("UAbilityTask_PlayMontageForMeshAndWaitForEvent call to PlayMontage failed!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("UARTAbilityTask_PlayMontageForMeshAndWaitForEvent called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "UGSAbilityTask_PlayMontageForMeshAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."
		       ), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			//ABILITY_LOG(Display, TEXT("%s: OnCancelled"), *GetName());
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UAT_PlayMontageMeshWaitForEvent::ExternalCancel()
{
	check(AbilitySystemComponent);

	OnAbilityCancelled();

	Super::ExternalCancel();
}

void UAT_PlayMontageMeshWaitForEvent::OnDestroy(bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage(OverrideBlendOutTimeForStopWhenEndAbility);
		}
	}

	UARTAbilitySystemComponent* ARTAbilitySystemComponent = GetTargetASC();
	if (ARTAbilitySystemComponent)
	{
		ARTAbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

bool UAT_PlayMontageMeshWaitForEvent::StopPlayingMontage(float OverrideBlendOutTime /*= -1.f*/)
{
	if (Mesh == nullptr)
	{
		return false;
	}

	UARTAbilitySystemComponent* ARTAbilitySystemComponent = GetTargetASC();
	if (!ARTAbilitySystemComponent)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	if (ARTAbilitySystemComponent && Ability)
	{
		if (ARTAbilitySystemComponent->GetAnimatingAbilityFromAnyMesh() == Ability
			&& ARTAbilitySystemComponent->GetCurrentMontageForMesh(Mesh) == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ARTAbilitySystemComponent->CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
			return true;
		}
	}

	return false;
}

FString UAT_PlayMontageMeshWaitForEvent::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability && Mesh)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay)
				                 ? MontageToPlay
				                 : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(
		TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay),
		*GetNameSafe(PlayingMontage));
}
