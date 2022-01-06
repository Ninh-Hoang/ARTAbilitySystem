// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "ARTOrderTypeWithIndex.generated.h"

/**
 * 
 */
/**
* An order type with an index.
*/

class UARTOrder;

USTRUCT(BlueprintType)
struct ART_API FARTOrderTypeWithIndex
{
	GENERATED_BODY()

	FARTOrderTypeWithIndex();
	FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex = -1);
	FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, FGameplayTagContainer InOrderTags);
	FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex = -1, FGameplayTagContainer InOrderTags = FGameplayTagContainer::EmptyContainer);
	/** Type of this order. */
	UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UARTOrder> OrderType;

	/** Additional index. This is needed for certain orders to differentiate. Default '-1'. */
	UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
	int32 Index;

	/** Additional GameplayTagContainer */
	UPROPERTY(Category = Order, EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer OrderTags;

	/**
	* Get a textual representation of this order.
	* @return A string describing the order.
	*/
	FString ToString() const;

	bool operator==(const FARTOrderTypeWithIndex& Other) const;
	bool operator!=(const FARTOrderTypeWithIndex& Other) const;
};
