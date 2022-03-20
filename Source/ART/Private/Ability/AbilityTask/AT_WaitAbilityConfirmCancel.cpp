// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_WaitAbilityConfirmCancel.h"

#include "AbilitySystemComponent.h"

UAbilityTask_WaitConfirmCancel* UAT_WaitAbilityConfirmCancel::WaitAbilityConfirmCancel(UGameplayAbility* OwningAbility)
{
	return NewAbilityTask<UAT_WaitAbilityConfirmCancel>(OwningAbility);
}

void UAT_WaitAbilityConfirmCancel::Activate()
{
	if (AbilitySystemComponent && Ability)
	{
		const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();

		
		if (Info->IsLocallyControlled())
		{
			// We have to wait for the callback from the AbilitySystemComponent.
			DelegateHandleConfirm = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UAbilityTask_WaitConfirmCancel::OnLocalConfirmCallback);
			DelegateHandleCancel = AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UAbilityTask_WaitConfirmCancel::OnLocalCancelCallback);

			Ability->OnWaitingForConfirmInputBegin();

			RegisteredCallbacks = true;
		}
		else
		{
			if (CallOrAddReplicatedDelegate(EAbilityGenericReplicatedEvent::GenericConfirm,  FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityTask_WaitConfirmCancel::OnConfirmCallback)) )
			{
				// GenericConfirm was already received from the client and we just called OnConfirmCallback. The task is done.
				return;
			}
			if (CallOrAddReplicatedDelegate(EAbilityGenericReplicatedEvent::GenericCancel,  FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAbilityTask_WaitConfirmCancel::OnCancelCallback)) )
			{
				// GenericCancel was already received from the client and we just called OnCancelCallback. The task is done.
				return;
			}
		}
	}
}

void UAT_WaitAbilityConfirmCancel::OnDestroy(bool AbilityEnding)
{
	if (RegisteredCallbacks && AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandleConfirm);
		AbilitySystemComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandleCancel);

		if (Ability)
		{
			Ability->OnWaitingForConfirmInputEnd();
		}
	}

	Super::OnDestroy(AbilityEnding);
}
