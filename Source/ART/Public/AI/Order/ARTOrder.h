// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ARTOrderErrorTags.h"
#include "ARTOrderGroupExecutionType.h"
#include "ARTOrderPreviewData.h"
#include "ARTOrderProcessPolicy.h"
#include "ARTOrderResult.h"
#include "ARTOrderTargetData.h"
#include "ARTOrderTagRequirements.h"
#include "ARTOrderTargetType.h"
#include "ARTOrder.generated.h"

/**
 * 
 */
class AActor;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTOrderCallback, EARTOrderResult, Result);

UCLASS(BlueprintType, Abstract, Blueprintable, Const)
class ART_API UARTOrder : public UObject
{
	GENERATED_BODY()

public:
	UARTOrder();

	/** Whether the specified actor can obey this kind of order. */
	virtual bool CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
	                          FARTOrderErrorTags* OutErrorTags = nullptr) const;

	/** Whether the specified actor and/or location is a valid target for this order. */
	virtual bool IsValidTarget(const AActor* OrderedActor, const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index,
	                           FARTOrderErrorTags* OutErrorTags = nullptr) const;

	/**
	* Creates individual target locations for the group of actors. The default implementation just returns the
	* specified 'TargetLocation' for each actor.
	*/
	virtual void CreateIndividualTargetLocations(const TArray<AActor*>& OrderedActors,
	                                             const FARTOrderTargetData& TargetData,
	                                             TArray<FVector2D>& OutTargetLocations) const;

	/**
	* Issues the specified actor to obey this order on the specified target. The callback must be invoked when the
	* order is finished.
	*/
	virtual void IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	                        const FGameplayTagContainer& OrderTags, int32 Index,
	                        FARTOrderCallback Callback, const FVector& HomeLocation);


	virtual void OrderCanceled(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	                           const FGameplayTagContainer& OrderTags, int32 Index) const;

	/** Gets the target type this order is using. */
	virtual EARTTargetType GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                     int32 Index) const;

	/** Whether this order type creates individual target locations for a group of actors */
	virtual bool IsCreatingIndividualTargetLocations(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                                 int32 Index) const;

	/** Gets the normal icon of the order. Can be shown in the UI. */
	virtual UTexture2D* GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                  int32 Index) const;

	/** Gets the hovered icon of the order. Can be shown in the UI. */
	virtual UTexture2D* GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                   int32 Index) const;

	/** Gets the pressed icon of the order. Can be shown in the UI. */
	virtual UTexture2D* GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                   int32 Index) const;

	/** Gets the disabled icon of the order. Can be shown in the UI. */
	virtual UTexture2D* GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                    int32 Index) const;

	/** Gets the Name of the order. Can be shown in the UI. */
	virtual FText GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const;

	/** Gets the Description of the order. Can be shown in the UI. */
	virtual FText GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const;

	/** Gets the index of the button when shown in the UI. */
	virtual int32 GetOrderButtonIndex() const;

	/** Checks whether to use a fixed index of the button when shown in the UI, instead of just lining it up among
	* others. */
	virtual bool HasFixedOrderButtonIndex() const;

	/** Gets details about the preview for the order while choosing a target. */
	virtual FARTOrderPreviewData GetOrderPreviewData(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                                 int32 Index) const;

	virtual void InitializePreviewActor(AARTOrderPreview* PreviewActor, const AActor* OrderedActor, const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index);
	
	/** Gets the tag requirements for this order that must be full filled to be issued. */
	virtual void GetTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
	                                FARTOrderTagRequirements& OutTagRequirements) const;

	/** Gets the tag requirements for this order that must be full filled to be successful. */
	virtual void GetSuccessTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                       int32 Index,
	                                       FARTOrderTagRequirements& OutTagRequirements) const;

	/**
	* Gets the required range between the ordered actor and the target that is needed to fulfill the order.
	* '0' value is returned if the order has no required range.
	*/
	virtual float GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                               int32 Index) const;

	/** Gets the specific acquisition radius for this order. */
	virtual bool GetAcquisitionRadiusOverride(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                          int32 Index,
	                                          float& OutAcquisitionRadius) const;

	/**
	* Gets a value that describes how an order is executed. This might determine how the order is displayed in the UI
	* and it determines how the order is handled by the order system.
	*/
	virtual EARTOrderProcessPolicy GetOrderProcessPolicy(const AActor* OrderedActor,
	                                                     const FGameplayTagContainer& OrderTags, int32 Index) const;

	/** Gets the order to issue instead if the player specified an invalid target for this one. */
	TSoftClassPtr<UARTOrder> GetFallbackOrder() const;

	/**
	* Evaluates the target and returns a score that can be used to compare the different targets. A higher score means
	* a better target for the order.
	*/
	virtual float GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	                             const FGameplayTagContainer& OrderTags, int32 Index) const;

	/**
	* Gets the group execution type of this order.
	*/
	virtual EARTOrderGroupExecutionType GetGroupExecutionType(const AActor* OrderedActor,
	                                                          const FGameplayTagContainer& OrderTags,
	                                                          int32 Index) const;

	/** Whether this order is an auto order for the human player. */
	virtual bool IsHumanPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                    int32 Index) const;

	/** When this order is an auto order, this value indicates whether it is active by default. */
	virtual bool GetHumanPlayerAutoOrderInitialState(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                                 int32 Index) const;

	/** Whether this order is an auto order for AI players. */
	virtual bool IsAIPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                 int32 Index) const;

	/** Whether this order allows auto orders when it is active. */
	virtual bool AreAutoOrdersAllowedDuringOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
									int32 Index) const;

protected:
	/** Tag requirements for an order that must be full filled to be issued. */
	UPROPERTY(Category = "Requirements", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FARTOrderTagRequirements TagRequirements;

	/** Tag requirements for an order that must be full filled to be to be successful. */
	UPROPERTY(Category = "Requirements", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FARTOrderTagRequirements SuccessTagRequirements;

	/**
	* Describes how an order is executed. This might determine how the order is displayed in the UI and it determines
	* how the order is handled by the order system.
	*/
	UPROPERTY(Category = "Policy", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTOrderProcessPolicy OrderProcessPolicy;

private:
	/** Order to issue instead if the player specified an invalid target for this one. */
	UPROPERTY(Category = "Policy", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSoftClassPtr<UARTOrder> FallbackOrder;
};
