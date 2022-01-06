#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARTOrderTagRequirements.generated.h"

/**
* Tag requirements for an order that must be full filled to be issued.
*/
USTRUCT(BlueprintType)
struct ART_API FARTOrderTagRequirements
{
	GENERATED_BODY()

	/** The order can only be issued if the ordered actor has all of these tags */
	UPROPERTY(Category = Tags, EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagContainer SourceRequiredTags;

	/** The order is blocked if the ordered actor has any of these tags */
	UPROPERTY(Category = Tags, EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagContainer SourceBlockedTags;

	/** The order can only be issued if the target actor has all of these tags */
	UPROPERTY(Category = Tags, EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagContainer TargetRequiredTags;

	/** The order is blocked if the target actor has any of these tags */
	UPROPERTY(Category = Tags, EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTagContainer TargetBlockedTags;

	/** Empty tag requirements. */
	static const FARTOrderTagRequirements EMPTY_TAG_REQUIREMENTS;
};
