#pragma once

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ARTItemModTypes.generated.h"

class UARTItemDefinition_Mod;

USTRUCT(BlueprintType)
struct ART_API FARTModModifierData
{
	GENERATED_BODY()
public:
	FARTModModifierData()
	{
		Value = 0.0f;
		ModifierOp = EGameplayModOp::Additive;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayModifier)
	FGameplayAttribute Attribute;

	/** The numeric operation of this modifier: Override, Add, Multiply, etc  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayModifier)
	TEnumAsByte<EGameplayModOp::Type> ModifierOp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayModifier)
	float Value;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayModifier)
	FGameplayTagQuery SlotApplicationRequirement;
};

FArchive& operator<<(FArchive& Ar, FARTModModifierData& ModData);