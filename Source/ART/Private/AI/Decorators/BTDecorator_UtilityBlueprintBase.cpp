// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTDecorator_UtilityBlueprintBase.h"

#include "AIController.h"
#include "BlueprintNodeHelpers.h"
#include "BehaviorTree/BehaviorTree.h"

UBTDecorator_UtilityBlueprintBase::UBTDecorator_UtilityBlueprintBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//NodeName = TEXT("UnknownUtilFunc");

	// all blueprint based nodes must create instances
	bCreateNodeInstance = true;
	bIsObservingBB = false;
	/*	@TODO: Look into how this is done in BTDecorator_BlueprintBase.

	UClass* StopAtClass = UBTDecorator_UtilityBlueprintBase::StaticClass();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
	BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);
	}
	*/
}

float UBTDecorator_UtilityBlueprintBase::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControlledPawn = nullptr;
	if (AIOwner)
	{
		ControlledPawn = AIOwner->GetPawn();
	}
	return CalculateUtility(AIOwner, ControlledPawn);
}

void UBTDecorator_UtilityBlueprintBase::SetOwner(AActor* InActorOwner)
{
	ActorOwner = InActorOwner;
	AIOwner = Cast< AAIController >(InActorOwner);
}

void UBTDecorator_UtilityBlueprintBase::InitializeProperties()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UClass* StopAtClass = UBTDecorator_UtilityBlueprintBase::StaticClass();
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);

		bIsObservingBB = BlueprintNodeHelpers::HasAnyBlackboardSelectors(this, StopAtClass);
	}
}


#if WITH_EDITOR
bool UBTDecorator_UtilityBlueprintBase::UsesBlueprint() const
{
	return true;
}
#endif // WITH_EDITOR

void UBTDecorator_UtilityBlueprintBase::PostInitProperties()
{
	Super::PostInitProperties();	
	InitializeProperties();

	if (bIsObservingBB)
	{
		bNotifyBecomeRelevant = true;
		bNotifyCeaseRelevant = true;
	}
}

void UBTDecorator_UtilityBlueprintBase::PostLoad()
{
	Super::PostLoad();

	if (GetFlowAbortMode() != EBTFlowAbortMode::None && bIsObservingBB)
	{
		ObservedKeyNames.Reset();
		UClass* StopAtClass = UBTDecorator_UtilityBlueprintBase::StaticClass();
		BlueprintNodeHelpers::CollectBlackboardSelectors(this, StopAtClass, ObservedKeyNames);
		ensure(ObservedKeyNames.Num() > 0);
	}
}

void UBTDecorator_UtilityBlueprintBase::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (Asset.BlackboardAsset)
	{
		BlueprintNodeHelpers::ResolveBlackboardSelectors(*this, *StaticClass(), *Asset.BlackboardAsset);
	}
}

FString UBTDecorator_UtilityBlueprintBase::GetStaticDescription() const
{
	FString ReturnDesc =
#if WITH_EDITORONLY_DATA
        CustomDescription.Len() ? CustomDescription :
#endif // WITH_EDITORONLY_DATA
        Super::GetStaticDescription();

	UBTDecorator_UtilityBlueprintBase* CDO = (UBTDecorator_UtilityBlueprintBase*)(GetClass()->GetDefaultObject());
	if (CDO)
	{
		UClass* StopAtClass = UBTDecorator_UtilityBlueprintBase::StaticClass();
		FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
		if (PropertyDesc.Len())
		{
			ReturnDesc += TEXT(":\n\n");
			ReturnDesc += PropertyDesc;
		}
	}

	return ReturnDesc;
}

void UBTDecorator_UtilityBlueprintBase::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray< FString >& Values) const
{
	UBTDecorator_UtilityBlueprintBase* CDO = (UBTDecorator_UtilityBlueprintBase*)(GetClass()->GetDefaultObject());
	if (CDO && CDO->PropertyData.Num())
	{
		BlueprintNodeHelpers::DescribeRuntimeValues(this, CDO->PropertyData, Values);
	}
}