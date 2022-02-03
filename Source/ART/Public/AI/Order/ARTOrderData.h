// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTOrder.h"
#include "ARTOrderData.generated.h"

class AActor;

/**
 * An order that can be issued to units and buildings.
 */
USTRUCT(BlueprintType)
struct ART_API FARTOrderData
{
    GENERATED_BODY()

    FARTOrderData();
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, AActor* InTarget);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, FVector2D InLocation);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, AActor* InTarget, FVector2D InLocation);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, AActor* InTarget);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FVector2D InLocation);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, AActor* InTarget, FVector2D InLocation);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags, AActor* InTarget);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags, FVector2D InLocation);
    FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags, AActor* InTarget, FVector2D InLocation);

    /** Type of this order. */
    UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<UARTOrder> OrderType;

    /** Indicates that 'Location' is valid and should be used by this order. */
    uint8 bUseLocation : 1;

    /**
     * Target location of the order. Use 'bUseLocation' to check if this location is really set and should be used.
     */
    UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bUseLocation))
    FVector2D Location;

    /** Target actor for this order. 'Null' if no order has been specified. */
    UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
    AActor* Target;

    /** Additional index. This is needed for certain orders to differentiate. Default '-1'. */
    UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
    int32 Index;

    /** Order Tags, mostly for ability. */
    UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer OrderTags;

    /**
     * Get a textual representation of this order.
     * @return A string describing the order.
     */
    FString ToString() const;

    bool operator==(const FARTOrderData& Other) const;

    bool operator!=(const FARTOrderData& Other) const;
};
