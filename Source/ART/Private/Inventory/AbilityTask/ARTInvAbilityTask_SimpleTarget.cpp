// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/AbilityTask/ARTInvAbilityTask_SimpleTarget.h"
#include "AbilitySystemComponent.h"

UAbilityTask_SimpleInvTarget::UAbilityTask_SimpleInvTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FGameplayAbilityTargetDataHandle UAbilityTask_SimpleInvTarget::GenerateTargetHandle()
{
	return FGameplayAbilityTargetDataHandle();
}

void UAbilityTask_SimpleInvTarget::HandleTargetData(const FGameplayAbilityTargetDataHandle& Data)
{
	
}

void UAbilityTask_SimpleInvTarget::HandleCancelled()
{

}

void UAbilityTask_SimpleInvTarget::Activate()
{
	//Create a network sync point here, as the server must wait for the predicting client to send us a prediction key for this event
	FScopedPredictionWindow ScopedWindow(AbilitySystemComponent, IsPredictingClient());

	FGameplayAbilityTargetDataHandle TargetDataHandle = GenerateTargetHandle();

	if (IsPredictingClient())
	{
		//Replicate up the client's targeting struct


		FGameplayTag ActivationTag;
		AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(),
			TargetDataHandle, ActivationTag, AbilitySystemComponent->ScopedPredictionKey);

		HandleTargetData(TargetDataHandle);
	}
	else
	{
		const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
		
		if(Ability->GetCurrentActorInfo()->OwnerActor.IsValid())
		{
			if (const APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get())
			{
				bool PCLocallyControlled =  PC->IsLocalController();
			}
			else 
			{
				AActor* const OwnerActorPtr = Ability->GetCurrentActorInfo()->OwnerActor.Get(/*bEvenIfPendingKill=*/ true);
				if (OwnerActorPtr)
				{
					bool OwnerActorAuthority = (OwnerActorPtr->GetLocalRole() == ROLE_Authority);
				}
				// Non-players are always locally controlled on the server
			}
		}
		//Hold onto the server target data, that way when the client data comes in we have this to compare against
		ServerTargetData = TargetDataHandle;

		//If we aren't a predicting client, but we are locally controlled, that means we are running in singleplayer or as the host of a listen server
		if (!IsLocallyControlled())
		{

			AbilitySystemComponent->AbilityTargetDataSetDelegate(GetAbilitySpecHandle(), GetActivationPredictionKey())
				.AddUObject(this, &UAbilityTask_SimpleInvTarget::OnTargetDataCallback);
			AbilitySystemComponent->AbilityTargetDataCancelledDelegate(GetAbilitySpecHandle(), GetActivationPredictionKey())
				.AddUObject(this, &UAbilityTask_SimpleInvTarget::OnTargetDataCancelled);

			AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(GetAbilitySpecHandle(), GetActivationPredictionKey());

			SetWaitingOnRemotePlayerData();
		}
		else
		{
			FGameplayTag ActivationTag;
			OnTargetDataCallback(TargetDataHandle, ActivationTag);
			EndTask();
		}

	}
}

void UAbilityTask_SimpleInvTarget::OnTargetDataCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	HandleTargetData(Data);

	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
  	
	EndTask();
}

void UAbilityTask_SimpleInvTarget::OnTargetDataCancelled()
{
	HandleCancelled();

	EndTask();
}