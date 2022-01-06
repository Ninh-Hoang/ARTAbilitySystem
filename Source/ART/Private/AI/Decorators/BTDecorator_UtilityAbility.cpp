// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTDecorator_UtilityAbility.h"

#include "AIController.h"
#include "Ability/ARTGameplayAbility.h"
#include "ARTCharacter/ARTCharacterBase.h"

UBTDecorator_UtilityAbility::UBTDecorator_UtilityAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Ability Utility";
}

FString UBTDecorator_UtilityAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Get Utility Score from Ability that: \n %s "), *CachedDescription);
}

void UBTDecorator_UtilityAbility::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);

	FBTUtilityAbilityDecoratorMemory* MyMemory = CastInstanceNodeMemory<FBTUtilityAbilityDecoratorMemory>(NodeMemory);

	Values.Add(FString::Printf(TEXT("Ability Utility Score: %s"), *FString::SanitizeFloat(MyMemory->UtilityScore)));
}

void UBTDecorator_UtilityAbility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	FBTUtilityAbilityDecoratorMemory* DecoratorMemory = (FBTUtilityAbilityDecoratorMemory*)NodeMemory;

	DecoratorMemory->UtilityScore = 0.0f;
}

uint16 UBTDecorator_UtilityAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTUtilityAbilityDecoratorMemory);
}

float UBTDecorator_UtilityAbility::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIOwner = OwnerComp.GetAIOwner();
	AARTCharacterBase* Avatar = AIOwner->GetPawn<AARTCharacterBase>();
	if(Avatar)
	{
		TArray<UARTGameplayAbility*> Abilities;
		Avatar->GetActiveAbilitiesWithTags(GameplayTagContainer, Abilities);
		if(Abilities.Num()>0)
		{
			FBTUtilityAbilityDecoratorMemory* MyMemory = (FBTUtilityAbilityDecoratorMemory*)NodeMemory;
			MyMemory->UtilityScore = Abilities[0]->ScoreAbilityUtility();
			return MyMemory->UtilityScore;
		}
	}
	return 0.0f;
}

#if WITH_EDITOR
void UBTDecorator_UtilityAbility::BuildDescription()
{
	CachedDescription = GameplayTagContainer.ToMatchingText(EGameplayContainerMatchType::All, IsInversed()).ToString();
}

void UBTDecorator_UtilityAbility::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property == NULL)
	{
		return;
	}

	BuildDescription();
}
#endif	// WITH_EDITOR