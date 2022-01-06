#pragma once

#include "AI/Decorators/BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityConstant.generated.h"

/**
* Constant utility function.
* The associated node's utility value is specified as a constant value.
*/
UCLASS(Meta = (DisplayName = "Constant Utility", Category = "Utility Functions"))
class ART_API UBTDecorator_UtilityConstant : public UBTDecorator_UtilityFunction
{
	GENERATED_BODY()

	public:
	UBTDecorator_UtilityConstant(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Utility")
	float UtilityValue;

	virtual FString GetStaticDescription() const override;
	
protected:
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};