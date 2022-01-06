// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTOrderWithBehavior.h"
#include "ARTOrderTargetType.h"
#include "ARTCharacterAIOrder.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintType, Abstract, Blueprintable)
class ART_API UARTCharacterAIOrder : public UARTOrderWithBehavior
{
	GENERATED_BODY()

	public:
	UARTCharacterAIOrder();

	//~ Begin UARTOrder Interface
	virtual EARTTargetType GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
	virtual bool IsCreatingIndividualTargetLocations(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
	virtual EARTOrderGroupExecutionType GetGroupExecutionType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
	//~ End UARTOrder Interface

	protected:
	/** The target type this order is using. */
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTTargetType TargetType;

	/**
	* To how many and which of the selected units should this order be issued to.
	*/
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTOrderGroupExecutionType GroupExecutionType;

	/** Whether this order type creates individual target locations for a group of actors */
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsCreatingIndividualTargetLocations;
};
