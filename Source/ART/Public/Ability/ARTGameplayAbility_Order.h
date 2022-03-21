// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/ARTGameplayAbility.h"
#include "AI/Order/ARTOrderGroupExecutionType.h"
#include "AI/Order/ARTOrderPreviewData.h"
#include "AI/Order/ARTOrderTagRequirements.h"
#include "AI/Order/ARTOrderTargetData.h"
#include "AI/Order/ARTOrderTargetType.h"
#include "ARTGameplayAbility_Order.generated.h"

/**
 * 
 */

/**
* Describes how an ability is executed. This might determine how the ability is displayed in the UI and it determines
* how the ability is handled by the order system.
* Note that this has nothing to do with the effects an ability might apply to a target. This has also nothing todo with
* the cooldown of the ability.
*/
UENUM(BlueprintType)
enum class EAbilityProcessPolicy : uint8
{
	/**
	* The ability has no duration. If it has been activated it is considered to be finished instantly. Note that this
	* does not necessarily mean that 'EndAbility' has already been called. It only means that the order system will not
	* wait until this ability is really finished before it proceeds and it will not actively cancel the ability if
	* another order is issued.
	*/
	INSTANT UMETA(DisplayName = "Instant"),

	/**
	* The ability is considered to have a duration. The order system will wait until the ability has been finished
	* ('EndAbility has been called') but will actively cancel the ability when another order is issued.
	*/
	CAN_BE_CANCELED UMETA(DisplayName = "Can be Canceled"),

	/**
	* The ability is considered to have a duration. The order system will wait until the ability has been finished
	* ('EndAbility has been called') regardless whether another order has been issued.
	*/
	CAN_NOT_BE_CANCELED UMETA(DisplayName = "Can not be Canceled"),

	// clang-format off

	/**
	* Same as 'CanBeCanceled' with the only difference that the ability can not be canceled as long an an ability task
	* is active (e.g. 'PlayMontageAndWaitWithNotify').
	*/
	CAN_BE_CANCELED_WHEN_NO_GAMEPLAY_TASK_IS_RUNNING UMETA(
		DisplayName = "Can be Canceled when no Gameplay Task is running")
	// clang-format on
};

UCLASS()
class ART_API UARTGameplayAbility_Order : public UARTGameplayAbility
{
	GENERATED_BODY()
	
public:
	UARTGameplayAbility_Order();

protected:
	/**
	* Describes how this ability is executed. This might determine how the ability is displayed in the UI and it
	* determines how the ability is handled by the order system. Note that this has nothing todo with the effects an
	* ability might apply to a target. This has also nothing todo with the cooldown of the ability.
	*/

	UPROPERTY(Category = "Ability|Order", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EAbilityProcessPolicy AbilityProcessPolicy;

	/**
	 * To how many and which of the selected units should this order be issued to.
	 */
	UPROPERTY(Category = "Ability|Order", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTOrderGroupExecutionType GroupExecutionType;

	/**
	 * The target type of this ability.
	 */

	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FScalableFloat AbilityBaseRange;

	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTTargetType TargetType;

	/** Details about the preview for this ability while choosing a target. */
	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FARTOrderPreviewData AbilityPreviewData;

	/** The specific acquisition radius for this ability. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly,
		meta = (AllowPrivateAccess = true, EditCondition = bIsAcquisitionRadiusOverridden))
	float AcquisitionRadiusOverride;

	/** Whether this ability uses a specific acquisition radius. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsAcquisitionRadiusOverridden; // InlineEditConditionToggle caused Editor crashes here.

	/** Auto order priority, lower means will be prioritized */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly,meta = (AllowPrivateAccess = true))
	int32 AutoOrderPriority;
	
	/** Whether this ability is an auto ability for the human player. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bHumanPlayerAutoAbility;

	/** When this ability is an auto ability, this value indicates whether it is active by default. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = true))
	bool bHumanPlayerAutoAutoAbilityInitialState;

	/** Whether this ability is an auto ability for AI players. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bAIPlayerAutoAbility;

	/** Whether this ability uses a specific target score. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsTargetScoreOverridden;

	/** Whether to show this ability as a default order in the UI, instead of as an ability. */
	UPROPERTY(Category = "Ability|Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bShouldShowAsOrderInUI;

public:
	/*
	 *Order related function
	 */
	/** Gets the target type of this ability. */
	EARTTargetType GetTargetType() const;

	/** Gets the ability process policy of this ability. */
	EAbilityProcessPolicy GetAbilityProcessPolicy() const;

	/** Gets the group execution type this ability. */
	EARTOrderGroupExecutionType GetGroupExecutionType() const;

	/** Gets details about the preview for this ability while choosing a target. */
	FARTOrderPreviewData GetAbilityPreviewData() const;

	/**
		 * Gets the minimum range between the caster and the target that is needed to activate the ability.
		 * '0' value is returned if the ability has no range.
		 */

	bool bHasBlueprintGetRange;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityRange",
		meta=(ScriptName="AbilityRange"))
	float K2_GetRange(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle) const;

	float GetRange(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const;

	/**
	 * Evaluates the target and returns a score that can be used to compare the different targets. A higher score means
	 * a better target for the ability.
	 */
	bool bHasBlueprintGetTargetScore;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityTargetScore",
		meta=(ScriptName="AbilityTargetScore"))
	float K2_GetTargetScore(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData, int32 Index) const;


	float GetTargetScore(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData, int32 Index) const;

	bool bHasBlueprintGetOrderTargetData;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityOrderTargetData",
		meta=(ScriptName="AbilityOrderTargetData"))
	bool K2_GetOrderTargetData(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle, FARTOrderTargetData& OrderTargetData) const;
	
	bool GetOrderTargetData(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, OUT FARTOrderTargetData* OrderTargetData = nullptr) const;

	/** Gets order tag requirements that corresponds with the tags of this ability. */
	void GetOrderTagRequirements(FARTOrderTagRequirements& OutTagRequirements) const;
	
	/** Gets the specific acquisition radius for this ability. */
	bool GetAcquisitionRadiusOverride(float& OutAcquisitionRadius) const;

	int32 GetAutoOrderPriority() const;

	/** Whether this ability is an auto ability for the human player. */
	bool IsHumanPlayerAutoAbility() const;

	/** When this ability is an auto ability, this value indicates whether it is active by default. */
	bool GetHumanPlayerAutoAutoAbilityInitialState() const;

	/** Whether this ability is an auto ability for AI players. */
	bool IsAIPlayerAutoAbility() const;

	/** Whether this ability uses a specific target score. */
	bool IsTargetScoreOverriden() const;

	UFUNCTION(BlueprintPure, Category="Order")
	FVector GetBlackboardOrderLocation() const;

	UFUNCTION(BlueprintPure, Category="Order")
	AActor* GetBlackboardOrderTarget() const;

	UFUNCTION(BlueprintPure, Category="Order")
	FVector GetBlackboardOrderHomeLocation() const;

	UFUNCTION(BlueprintPure, Category="Order")
	float GetBlackboardOrderRange() const;
};
