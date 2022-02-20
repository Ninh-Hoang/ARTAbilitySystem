// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitItemSwitch.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitItemSwitchDelegate);

UCLASS()
class ART_API UAbilityTask_WaitItemSwitch : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_WaitItemSwitch(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static 	UAbilityTask_WaitItemSwitch* WaitActiveItemSwitch(UGameplayAbility* OwningAbility, int32 ItemIndex);

	
	virtual void Activate() override;
	virtual void BeginDestroy() override;


	virtual void DoItemEquip();

	UPROPERTY(BlueprintAssignable)
	FWaitItemSwitchDelegate OnItemSwitched;
	
protected: 

	UPROPERTY()
	class UARTInventoryComponent_Active* Inventory; 
	int32 ItemIndex;

	FTimerHandle TimerHandle;
	
};
