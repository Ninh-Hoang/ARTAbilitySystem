// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_ActivateAbility.h"

#include "Ability/ARTGameplayAbility.h"
#include "AI/Order/ARTOrderComponent.h"
#include "ARTCharacter/ARTCharacterBase.h"

UBTTask_ActivateAbility::UBTTask_ActivateAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Activate Gameplay Ability";
	bCreateNodeInstance = true;
	
	UseOrderTags = false;
	BroadcastResultToController = false;
	InstantExecute = false;
}


EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AIController = Cast<AARTAIController>(OwnerComp.GetOwner());
	APawn* MyPawn = AIController ? AIController->GetPawn() : NULL;

	if (!MyPawn)
	{
		return EBTNodeResult::Failed;
	}
	if (AARTCharacterBase* AvatarActor = Cast<AARTCharacterBase>(MyPawn))
	{
		if (UseOrderTags)
		{
			UARTOrderComponent* OrderComp = AvatarActor->FindComponentByClass<UARTOrderComponent>();
			GameplayTagContainer = OrderComp->GetCurrentOrderTagContainer();
		}

		ASC = AvatarActor->GetAbilitySystemComponent();
		TArray<FGameplayAbilitySpec*> SpecArray;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, SpecArray, false);
		if(SpecArray.Num() <1 ) return EBTNodeResult::Failed;

		TArray<UGameplayAbility*> InstanceAbilities = SpecArray[0]->GetAbilityInstances();
		if(InstanceAbilities.Num() < 1) return EBTNodeResult::Failed;
		
		ActiveAbility = SpecArray[0]->GetAbilityInstances()[0];
		
		OnAbilityEndHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ActivateAbility::OnAbilityEnded);
		
		bool Activated = AvatarActor->ActivateAbilitiesWithTags(GameplayTagContainer, false);

		if (!Activated)
		{
			if(BroadcastResultToController) AIController->BehaviorTreeEnded(EBTNodeResult::Failed);
			return EBTNodeResult::Failed;
		}
		if (InstantExecute) return EBTNodeResult::Succeeded;


		MyOwnerComp = &OwnerComp;
		return EBTNodeResult::InProgress;
	}
	if(BroadcastResultToController) AIController->BehaviorTreeEnded(EBTNodeResult::Failed);
	return EBTNodeResult::Failed;
}


void UBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	if(BroadcastResultToController) AIController->BehaviorTreeEnded(TaskResult);
	ASC->OnAbilityEnded.Remove(OnAbilityEndHandle);
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_ActivateAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ASC->OnAbilityEnded.Remove(OnAbilityEndHandle);
	if(BroadcastResultToController) AIController->BehaviorTreeEnded(EBTNodeResult::Aborted);
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateAbility::OnAbilityEnded(const FAbilityEndedData& Data)
{
	if (Data.AbilityThatEnded == ActiveAbility)
	{
		ASC->OnAbilityEnded.Remove(OnAbilityEndHandle);
		UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(GetOuter());
		const EBTNodeResult::Type NodeResult = Data.bWasCancelled ? EBTNodeResult::Aborted : EBTNodeResult::Succeeded;
		if(BroadcastResultToController) AIController->BehaviorTreeEnded(NodeResult);
		FinishLatentTask(*OwnerComp, NodeResult);
	}
}

FString UBTTask_ActivateAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate Ability that: \n %s "), *CachedDescription);
}

#if WITH_EDITOR
FName UBTTask_ActivateAbility::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.Icon");
}

void UBTTask_ActivateAbility::BuildDescription()
{
	CachedDescription = GameplayTagContainer.ToMatchingText(EGameplayContainerMatchType::All, false).ToString();
}

void UBTTask_ActivateAbility::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property == NULL)
	{
		return;
	}

	BuildDescription();
}
#endif	// WITH_EDITOR
