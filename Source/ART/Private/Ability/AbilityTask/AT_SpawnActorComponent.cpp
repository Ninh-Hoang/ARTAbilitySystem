// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_SpawnActorComponent.h"

UAT_SpawnActorComponent::UAT_SpawnActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAT_SpawnActorComponent::Activate()
{
	USceneComponent* CreatedComp = NewObject<USceneComponent>(ActorToAttachTo, ActorComponentClass, ComponentName);
	if (CreatedComp && ActorToAttachTo)
	{
		FAttachmentTransformRules AttachRule = FAttachmentTransformRules::KeepRelativeTransform;
		CreatedComp->RegisterComponent();
		CreatedComp->AttachToComponent(ActorToAttachTo->GetRootComponent(), AttachRule);
		OnSuccess.Broadcast(CreatedComp);
	}
	else
	{
		OnFail.Broadcast(CreatedComp);
	}
	EndTask();
}

UAT_SpawnActorComponent* UAT_SpawnActorComponent::SpawnActorComponent(UGameplayAbility* OwningAbility,
                                                                      TSubclassOf<USceneComponent>
                                                                      InActorComponentClass, FName InComponentName,
                                                                      AActor* InActorToAttachTo)
{
	UAT_SpawnActorComponent* MyObj = NewAbilityTask<UAT_SpawnActorComponent>(OwningAbility);
	MyObj->ActorComponentClass = InActorComponentClass;
	MyObj->ComponentName = InComponentName;
	MyObj->ActorToAttachTo = InActorToAttachTo;
	return MyObj;
}
