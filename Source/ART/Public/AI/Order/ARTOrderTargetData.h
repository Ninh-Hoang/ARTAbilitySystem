#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARTOrderTargetData.generated.h"

/** Targeting data for an order. */
USTRUCT(BlueprintType)
struct ART_API FARTOrderTargetData
{
	GENERATED_BODY()

	FARTOrderTargetData();
	FARTOrderTargetData(AActor* InActor, const FVector2D InLocation, const FGameplayTagContainer& InTargetTags);

	/** The target actor. */
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadWrite)
	AActor* Actor;

	/** The target location. */
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadWrite)
	FVector2D Location;

	/** The target tags. */
	UPROPERTY(Category = ART, EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagContainer TargetTags;

	/**
	* Get a textual representation of this order target data.
	* @return A string describing the order target data.
	*/
	FString ToString() const;
};
