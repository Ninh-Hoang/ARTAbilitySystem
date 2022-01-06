// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTOrderData.h"
#include "ARTOrderTypeWithIndex.h"
#include "ARTAutoOrderComponent.generated.h"


UCLASS(meta=(BlueprintSpawnableComponent))
class ART_API UARTAutoOrderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UARTAutoOrderComponent();

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Whether the specified order is a togglable auto order for the human player. */
    UFUNCTION(Category = Order, BlueprintPure)
    bool IsHumanPlayerAutoOrder(const FARTOrderTypeWithIndex& Order);

    /** Sets whether the specified auto order is enabled to be issued automatically. */
    UFUNCTION(Category = Order, BlueprintCallable)
    void SetHumanPlayerAutoOrderState(const FARTOrderTypeWithIndex& Order, bool bEnable);

    /** Gets whether the the specified auto order is enabled to be issued automatically. */
    UFUNCTION(Category = Order, BlueprintPure)
    bool GetHumanPlayerAutoOrderState(const FARTOrderTypeWithIndex& Order);

    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent Interface

    void CheckAutoOrders();

private:
    UFUNCTION()
    void OnOrderChanged(const FARTOrderData& NewOrder);

    UFUNCTION()
    void OnOwnerChanged(APlayerState* PreviousOwner, APlayerState* NewOwner);

    bool IssueAutoOrder(const FARTOrderTypeWithIndex& Order);
    float GetAcquisitionRadius(const FARTOrderTypeWithIndex& Order);

    UFUNCTION()
    void OnAutoOrderAdded(const FARTOrderTypeWithIndex& AutoOrder);

    UFUNCTION()
    void OnAutoOrderRemoved(const FARTOrderTypeWithIndex& AutoOrder);

    /** Contains all orders that may be issued automatically with their associated index. */
    TArray<FARTOrderTypeWithIndex> Orders;

    /** Whether an auto order is a togglable auto order for the human player.  */
    // TODO: Are these values really needed. The 'Orders' array should only contain orders that can be issued
    // automatically by human players. It should only contain more orders for AI players.
    TArray<bool> HumanPlayerAutoOrders;

    /** Whether an order should automatically be activated by AI players.  */
    TArray<bool> AIPlayerAutoOrders;

    /** Whether an auto order is enabled to be issued automatically. */
    UPROPERTY(replicated)
    TArray<bool> HumanPlayerAutoOrderStates;

    bool bCheckAutoOrders;
};
