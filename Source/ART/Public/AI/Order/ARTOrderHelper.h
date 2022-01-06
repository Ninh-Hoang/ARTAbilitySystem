// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ARTOrder.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ARTOrderData.h"
#include "ARTOrderTypeWithIndex.h"
#include "ARTOrderHelper.generated.h"

/**
 * 
 */
class AActor;
class UTexture2D;
class UARTOrderWithBehavior;

UCLASS()
class ART_API UARTOrderHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
    /** Whether the specified actor can obey this kind of order. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool CanObeyOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Whether the specified actor can obey this kind of order. */
    UFUNCTION(Category = "Order", BlueprintPure, DisplayName = "Can Obey Order (With Error Tags)")
    static bool K2_CanObeyOrderWithErrorTags(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                             const FGameplayTagContainer& OrderTags, int32 Index, FARTOrderErrorTags& OutErrorTags);

    /** Whether the specified actor can obey this kind of order. */
    static bool CanObeyOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                             FARTOrderErrorTags* OutErrorTags);

    /** Whether the specified actor and/or location is a valid target for this order. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool IsValidTarget(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                              const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Whether the specified actor and/or location is a valid target for this order. */
    UFUNCTION(Category = "Order", BlueprintPure, DisplayName = "Is Valid Target (With Error Tags)")
    static bool K2_IsValidTargetWithErrorTags(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                              const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index,
                                              FARTOrderErrorTags& OutErrorTags);

    /** Whether the specified actor and/or location is a valid target for this order. */
    static bool IsValidTarget(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                              const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index, FARTOrderErrorTags* OutErrorTags);

    /** Creates individual target locations for the group of actors. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static void CreateIndividualTargetLocations(TSoftClassPtr<UARTOrder> OrderType,
                                                const TArray<AActor*>& OrderedActors,
                                                const FARTOrderTargetData& TargetData,
                                                TArray<FVector2D>& OutTargetLocations);

    /** Issues the specified actor to obey this order on the specified target. */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static void IssueOrder(AActor* OrderedActor, const FARTOrderData& Order);

    /** Clears the order of the specified actor. Should probably only be used if queuing orders immediately after. */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static void ClearOrderQueue(AActor* OrderedActor);

    /** Enqueues an order that will be issued to the specified actor if all its other orders has succeeded. */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static void EnqueueOrder(AActor* OrderedActor, const FARTOrderData& Order);

    /** InseART an order that will be issued when this current order of the specified actor has succeeded. */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static void InsertOrderAfterCurrentOrder(AActor* OrderedActor, const FARTOrderData& Order);

    /** Gets the target type the specified order is using. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static EARTTargetType GetTargetType(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                        const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Whether this order type creates individual target locations for a group of actors */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool IsCreatingIndividualTargetLocations(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                    const FGameplayTagContainer& OrderTags, int32 Index);

    /** Gets the behavior tree that should run in order to obey this order. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static class UBehaviorTree* GetBehaviorTree(TSoftClassPtr<UARTOrderWithBehavior> OrderType);

    /** Whether to restart the behaviour tree whenever a new order of the specified type is issued. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool ShouldRestartBehaviourTree(TSoftClassPtr<UARTOrderWithBehavior> OrderType);

    /** Creates order target data using the specified target actor. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static FARTOrderTargetData CreateOrderTargetData(const AActor* OrderedActor, AActor* TargetActor,
                                                     const FVector2D& TargetLocation = FVector2D::ZeroVector);

    /** Gets the normal icon of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static UTexture2D* GetNormalIcon(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                     const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the hovered icon of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static UTexture2D* GetHoveredIcon(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                      const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the pressed icon of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static UTexture2D* GetPressedIcon(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                      const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the disabled icon of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static UTexture2D* GetDisabledIcon(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                       const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the name of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static FText GetName(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the name of the order. Can be shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static FText GetDescription(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the index of the button when shown in the UI. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static int32 GetOrderButtonIndex(TSoftClassPtr<UARTOrder> OrderType);

    /** Checks whether to use a fixed index of the button when shown in the UI, instead of just lining it up among
     * others. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool HasFixedOrderButtonIndex(TSoftClassPtr<UARTOrder> OrderType);

    /** Gets details about the preview for the order while choosing a target. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static FARTOrderPreviewData GetOrderPreviewData(TSoftClassPtr<UARTOrder> OrderType,
                                                    const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets details about the preview for the order while choosing a target. */
    UFUNCTION(Category = "Order", BlueprintPure)
 static void InitializePreviewActor(TSoftClassPtr<UARTOrder> OrderType, AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index);

    
    /** Gets the tag requirements for this order that must be full filled to be issued. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static void GetOrderTagRequirements(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                                        FARTOrderTagRequirements& OutTagRequirements);

    /** Gets the tag requirements for this order that must be full filled to be successful. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static void GetOrderSuccessTagRequirements(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                               const FGameplayTagContainer& OrderTags, int32 Index, FARTOrderTagRequirements& OutTagRequirements);

    /**
     * Gets the required range between the ordered actor and the target that is needed to fulfill the order.
     * '0' value is returned if the order has no required range.
     */
    UFUNCTION(Category = "Order", BlueprintPure)
    static float GetOrderRequiredRange(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                       const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /**
     * Gets the specified order specific acquisition radius for this order and returns whether the specified order uses
     * a specific acquisition radius.
     */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool GetAcquisitionRadiusOverride(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                             const FGameplayTagContainer& OrderTags, int32 Index, float& OutAcquisitionRadius);

    /**
     * Gets a value that describes how the specified order is executed. This might determine how the order is displayed
     * in the UI and it determines how the order is handled by the order system.
     */
    UFUNCTION(Category = "Order", BlueprintPure)
    static EARTOrderProcessPolicy GetOrderProcessPolicy(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                        const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the order to issue instead if the player specified an invalid target for the specified one. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static TSoftClassPtr<UARTOrder> GetFallbackOrder(TSoftClassPtr<UARTOrder> OrderType);

    /**
     * Evaluates the target and returns a score that can be used to compare the different targets. A higher score means
     * a better target for the order.
     */
    UFUNCTION(Category = "Order", BlueprintPure)
    static float GetOrderTargetScore(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                     const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Whether the specified order is an auto order for the human player. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool IsHumanPlayerAutoOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                       const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Whether the specified order is an auto order for AI players. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool IsAIPlayerAutoOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** When the specified order is an auto order, this value indicates whether it is active by default. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool GetHumanPlayerAutoOrderInitialState(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                    const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Sets whether the specified auto order is enabled to be issued automatically. */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static void SetHumanPlayerAutoOrderState(const AActor* OrderedActor, const FARTOrderTypeWithIndex& Order,
                                             bool bEnable);

    /** Whether the specified order order allows auto orders when it is active. */
    UFUNCTION(Category = "Order", BlueprintPure)
 static bool AreAutoOrdersAllowedDuringOrder(TSoftClassPtr<UARTOrder> OrderType,
                                                               const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /**
     * Whether the specified order can be considered as succeeded in regard to the specified ordered actor and order
     * target.
     */
    UFUNCTION(Category = "Order", BlueprintPure)
    static bool CanOrderBeConsideredAsSucceeded(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                const FARTOrderTargetData& TargetData, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Gets the group execution type of the specified order. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static EARTOrderGroupExecutionType GetOrderGroupExecutionType(TSoftClassPtr<UARTOrder> OrderType,
                                                                  const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index = -1);

    /** Checks whether the specified actor can apply the passed order to its target. */
    UFUNCTION(Category = "Order", BlueprintPure)
    static FARTOrderErrorTags CheckOrder(AActor* OrderedActor, const FARTOrderData& Order);

    /**
     * Checks if their is an actor that has a hostile relationship with the ordered actor inside the specified
     * acquisition radius.
     */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static bool IsEnemyInAcquisitionRadius(const AActor* OrderedActor, float AcquisitionRadius);

    /**
     * Finds the best matching target for the specified order inside the specified acquisition radius.
     * @param OrderType                 The order type.
     * @param OrderedActor              The ordered actor
     * @param Index                     Order index. This is needed for certain orders to differentiate. Default '-1'.
     * @param AcquisitionRadius         Max distance from the ordered actor to a potential target.
     * @param OutScore                  Score of the returned target if a target was found.
     * @return                          The target or 'nullptr' if no target was found.
     */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static AActor* FindTargetForOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
                                      float AcquisitionRadius, float& OutScore);

    /**
     * Finds the best matching target for the specified order inside the specified acquisition radius and inside the
     * specified chase distance.
     * @param OrderType                 The order type.
     * @param OrderedActor              The ordered actor
     * @param Index                     Order index. This is needed for certain orders to differentiate. Default '-1'.
     * @param AcquisitionRadius         Max distance from the ordered actor to a potential target.
     * @param ChaseDistance             Max distance from the 'OrderedActorHomeLocation' to a potential target.
     * @param OrderedActorHomeLocation  Home location of the 'OrderedActor'. Used in conjunction with 'ChaseDistance'
     * @param OutScore                  Score of the returned target if a target was found.
     * @return                          The target or 'nullptr' if no target was found.
     */
    UFUNCTION(Category = "Order", BlueprintCallable)
    static AActor* FindTargetForOrderInChaseDistance(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                     const FGameplayTagContainer& OrderTags, int32 Index, float AcquisitionRadius, float ChaseDistance,
                                                     const FVector& OrderedActorHomeLocation, float& OutScore);

    /**
     * Finds the most suitable actor to obey the specified order.
     * @param OrderType                 The order type.
     * @param OrderedActors             The actors to check.
     * @param TargetData                The target for the order.
     * @param Index                     Order index. This is needed for certain orders to differentiate. Default '-1'.
     * @return                          The target or 'nullptr' if no target was found.
     */
    static AActor* FindMostSuitableActorToObeyTheOrder(TSoftClassPtr<UARTOrder> OrderType,
                                                       const TArray<AActor*> OrderedActors,
                                                       const FARTOrderTargetData TargetData, const FGameplayTagContainer& OrderTags, int32 Index);

    /**
     * Finds all actors inside the specified acquisition radius.
     * @param WorldContextObject        World context.
     * @param AcquisitionRadius         Max distance from the ordered actor to a potential target.
     * @param OrderedActorLocation      Current location of the ordered actor.
     * @param OutActorsInRange          All found actors in range.
     */
    static void FindActors(UObject* WorldContextObject, float AcquisitionRadius, const FVector& OrderedActorLocation,
                           TArray<AActor*>& OutActorsInRange);

    /**
     * Finds all actors inside the specified acquisition radius and inside the specified chase distance.
     * @param WorldContextObject        World context.
     * @param AcquisitionRadius         Max distance from the ordered actor to a potential target.
     * @param ChaseDistance             Max distance from the 'OrderedActorHomeLocation' to a potential target.
     * @param OrderedActorLocation      Current location of the ordered actor.
     * @param OrderedActorHomeLocation  Home location of the ordered actor'. Used in conjunction with 'ChaseDistance'
     * @param OutActorsInRange          All found actors in range.
     */
    static void FindActorsInChaseDistance(UObject* WorldContextObject, float AcquisitionRadius, float ChaseDistance,
                                          const FVector& OrderedActorLocation, const FVector& OrderedActorHomeLocation,
                                          TArray<AActor*>& OutActorsInRange);

    /**
     * Finds the best matching target for the specified order.
     * @param OrderType                 The order type.
     * @param OrderedActor              The ordered actor
     * @param Index                     Order index. This is needed for certain orders to differentiate. Default '-1'.
     * @param Targets                   All potential target actors.
     * @param OutScore                  Score of the returned target if a target was found.
     * @return                          The target or 'nullptr' if no target was found.
     */
    static AActor* FindBestScoredTargetForOrder(TSoftClassPtr<UARTOrder> OrderType, const AActor* OrderedActor,
                                                const TArray<AActor*> Targets, const FGameplayTagContainer& OrderTags, int32 Index, float& OutScore);
};
