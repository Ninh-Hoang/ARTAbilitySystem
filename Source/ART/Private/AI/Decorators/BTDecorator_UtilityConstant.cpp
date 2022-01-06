
#include "AI/Decorators/BTDecorator_UtilityConstant.h"


UBTDecorator_UtilityConstant::UBTDecorator_UtilityConstant(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Constant Utility";
	UtilityValue = 0.0f;
}

FString UBTDecorator_UtilityConstant::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility Score: %f."), UtilityValue);
}

float UBTDecorator_UtilityConstant::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return UtilityValue;
}


