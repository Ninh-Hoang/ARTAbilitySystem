// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_WaitDelayOneFrame.h"
#include "TimerManager.h"

UAT_WaitDelayOneFrame::UAT_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAT_WaitDelayOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAT_WaitDelayOneFrame::OnDelayFinish);
}

UAT_WaitDelayOneFrame* UAT_WaitDelayOneFrame::WaitDelayOneFrame(UGameplayAbility* OwningAbility)
{
	UAT_WaitDelayOneFrame* MyObj = NewAbilityTask<UAT_WaitDelayOneFrame>(OwningAbility);
	return MyObj;
}

void UAT_WaitDelayOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	EndTask();
}
