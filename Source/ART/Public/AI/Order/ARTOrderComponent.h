// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTOrderData.h"
#include "ARTOrderResult.h"
#include "ARTOrderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTOrderComponentOrderEnqueuedSignature, const FARTOrderData&, Order);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FARTOrderComponentOrderQueueClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FARTOrderComponentOrderChangedSignature, const FARTOrderData&, NewOrder);

UCLASS()
class ART_API UARTOrderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UARTOrderComponent(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Event when the actor has received a new order. */
	virtual void NotifyOnOrderChanged(const FARTOrderData& NewOrder);

	/** Event when the actor has received a new order. */
	UPROPERTY(BlueprintAssignable, Category = "RTS")
	FARTOrderComponentOrderChangedSignature OnOrderChanged;

	/** Issues this unit to obey the specified order. */
	void IssueOrder(const FARTOrderData& Order);

	/** Enqueues an order that will be issued to the unit if all other orders has succeeded. */
	void EnqueueOrder(const FARTOrderData& Order);

	/** Inserts an order that will be issued when this current order has succeeded. */
	void InsertOrderAfterCurrentOrder(const FARTOrderData& Order);

	/** Clears the order queue. */
	void ClearOrderQueue();

	/**
	* Inserts an order that will be issued immediately. The current order will be ordered again when this order
	* finishes.
	*/
	void InsertOrderBeforeCurrentOrder(const FARTOrderData& Order);

	/** Gets the type of the current order of this pawn. */
	UFUNCTION(Category = Order, BlueprintPure)
	TSoftClassPtr<UARTOrder> GetCurrentOrderType() const;

	/** Checks whether the pawn is idle, or has any orders. */
	UFUNCTION(Category = RTS, BlueprintPure)
	bool IsIdle() const;

	FARTOrderData GetCurrentOrderData() const;

	TArray<FARTOrderData> GetCurrentOrderDataQueue() const;

	/** Gets the target actor of the current order of this pawn. */
	UFUNCTION(Category = Order, BlueprintPure)
	AActor* GetCurrentOrderTargetActor() const;

	/** Gets the target location of the current order of this pawn. */
	UFUNCTION(Category = Order, BlueprintPure)
	FVector2D GetCurrentOrderTargetLocation() const;

	/** Gets the target index of the current order of this pawn. */
	UFUNCTION(Category = Order, BlueprintPure)
	int32 GetCurrentOrderTargetIndex() const;

	/** Gets the order tags. */
	UFUNCTION(Category = Order, BlueprintPure)
	FGameplayTagContainer GetCurrentOrderTagContainer() const;

	/** Event when a new order has been enqueued. */
	UPROPERTY(BlueprintAssignable, Category = "Order")
	FARTOrderComponentOrderEnqueuedSignature OnOrderEnqueued;

	/** Event when the order queue has been cleared. */
	UPROPERTY(BlueprintAssignable, Category = "Order")
	FARTOrderComponentOrderQueueClearedSignature OnOrderQueueCleared;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY()
	class UARTSelectComponent* SelectComponent; 

private:
	UPROPERTY(BlueprintReadOnly, Category = "Order", ReplicatedUsing = ReceivedCurrentOrder,
			meta = (AllowPrivateAccess = true))
	FARTOrderData CurrentOrder;

	UPROPERTY(BlueprintReadOnly, Category = "Order", ReplicatedUsing = ReceivedCurrentOrder,
			meta = (AllowPrivateAccess = true))
	FARTOrderData LastOrder;

	UPROPERTY(BlueprintReadOnly, Category = "Order", ReplicatedUsing = ReceivedOrderQueue,
			meta = (AllowPrivateAccess = true))
	TArray<FARTOrderData> OrderQueue;

	UPROPERTY()
	TSoftClassPtr<UARTOrder> StopOrder;

	UPROPERTY()
	TArray<AActor*> OrderPreviewActors;

	/** Class of the preview actor that is used to show the target location of an order. */
	UPROPERTY(Category = RTS, BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> OrderPreviewActorClass;

	/** Order type that is used to begin the construction of a building. */
	UPROPERTY(Category = RTS, BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSoftClassPtr<UARTOrder> BeginConstructionOrder;

	/**
	* The handles of the delegates that are registered on the ability system of the actor owner to be able to abort
	* the order if the requirements are nor longer fulfilled.
	*/
	TMap<FGameplayTag, FDelegateHandle> RegisteredOwnerTagEventHandles;

	/**
	* The handles of the delegates that are registered on the ability system of the actor target to be able to abort
	* the order if the requirements are nor longer fulfilled.
	*/
	TMap<FGameplayTag, FDelegateHandle> RegisteredTargetTagEventHandles;

	/** Last order home location if set. */
	FVector LastOrderHomeLocation;

	/** Indicates whether 'LastOrderHomeLocation' has a valid value. */
	bool bIsHomeLocationSet;

	/** Store the current order of the actor. */
	void SetCurrentOrder(FARTOrderData NewOrder);

	UFUNCTION()
	void ReceivedCurrentOrder();

	UFUNCTION()
	void ReceivedOrderQueue();

	void ObeyOrder(const FARTOrderData& Order);
	bool CheckOrder(const FARTOrderData& Order) const;
	void LogOrderErrorMessage(const FString& Message, const FARTOrderErrorTags& OrderErrorTags) const;

	UFUNCTION()
	void OnOrderEndedCallback(EARTOrderResult OrderResult);

	void OrderEnded(EARTOrderResult OrderResult);
	void OrderCanceled();
	void RegisterTagListeners(const FARTOrderData& Order);
	void UnregisterTagListeners(const FARTOrderData& Order);

	UFUNCTION()
	void OnTargetTagsChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnOwnerTagsChanged(const FGameplayTag Tag, int32 NewCount);

	void ObeyStopOrder();

	AActor* CreateOrderPreviewActor(const FARTOrderData& Order);

	UFUNCTION()
	void OnSelected();

	UFUNCTION()
	void OnDeselected();

	void UpdateOrderPreviews();
};
