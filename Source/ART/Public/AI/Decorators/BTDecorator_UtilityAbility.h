// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Decorators/BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityAbility.generated.h"

struct FBTUtilityAbilityDecoratorMemory
{
	float UtilityScore;
};
/**
* Ability Utility function.
* Get Ability ScoreUtility() return.
*/
UCLASS(Meta = (DisplayName = "Ability Utility", Category = "Utility Functions"))
class ART_API UBTDecorator_UtilityAbility : public UBTDecorator_UtilityFunction
{
	GENERATED_BODY()
	
	UBTDecorator_UtilityAbility(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, Category = "Utility")
	FGameplayTagContainer GameplayTagContainer;

	/** cached description */
	UPROPERTY()
	FString CachedDescription;

	virtual FString GetStaticDescription() const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual uint16 GetInstanceMemorySize() const override;
protected:
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

#if WITH_EDITOR
	/** describe decorator and cache it */
	virtual void BuildDescription();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
