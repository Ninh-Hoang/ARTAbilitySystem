// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_SpawnActorComponent.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnActorComponentDelegate, USceneComponent*, OutComponent);

UCLASS()
class ART_API UAT_SpawnActorComponent : public UAbilityTask
{
	GENERATED_BODY()

	//constructor override
	UAT_SpawnActorComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FSpawnActorComponentDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FSpawnActorComponentDelegate OnFail;

	TSubclassOf<USceneComponent> ActorComponentClass;

	FName ComponentName;

	AActor* ActorToAttachTo;


	/**
	 * Spawn an Actor Component and Attach it to an actor
	 *
	 * @param TaskInstanceName Set to override the name of this task, for later querying
	 * @param MontageToPlay The montage to play on the character
	 * @param EventTags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * @param Rate Change to play the montage faster or slower
	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
	 */
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf =
		"OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_SpawnActorComponent* SpawnActorComponent(
		UGameplayAbility* OwningAbility,
		TSubclassOf<USceneComponent> InActorComponentClass,
		FName InComponentName,
		AActor* InActorToAttachTo);
private:
};
