// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Decorators/BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityBlueprintBase.generated.h"

/*
* @TODO: Check PropertyData from other blueprint base because this one does not have it
*/
UCLASS(Abstract, Blueprintable)
class ART_API UBTDecorator_UtilityBlueprintBase : public UBTDecorator_UtilityFunction
{
	GENERATED_UCLASS_BODY()

    /* UBTDecorator_UtilityFunction interface */
    virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	virtual void SetOwner(AActor* ActorOwner) override;

	/** initialize data about blueprint defined properties */
	void InitializeProperties();

#if WITH_EDITOR
	virtual bool UsesBlueprint() const override;
#endif

protected:
	/** Cached AIController owner of BehaviorTreeComponent. */
	UPROPERTY(Transient)
	AAIController* AIOwner;

	/** Cached actor owner of BehaviorTreeComponent. */
	UPROPERTY(Transient)
	AActor* ActorOwner;
	
	/** blackboard key names that should be observed */
	UPROPERTY()
	TArray<FName> ObservedKeyNames;
	
	/** properties with runtime values, stored only in class default object */
	TArray<FProperty*> PropertyData;

	/** gets set to true if decorator declared BB keys it can potentially observe */
	UPROPERTY()
	uint32 bIsObservingBB : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Description)
	FString CustomDescription;
#endif // WITH_EDITORONLY_DATA

	protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Utility)
    float CalculateUtility(AAIController* OwnerController, APawn* ControlledPawn) const;

public:
	/** setup node name */
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual FString GetStaticDescription() const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
};


