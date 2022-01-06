// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Tasks/BTTask_GameplayTagBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ARTCharacter/AI/ARTAIController.h"

#include "BTTask_ActivateAbility.generated.h"

/**
 * Activate Gameplay Ability
 */
UCLASS()
class ART_API UBTTask_ActivateAbility : public UBTTask_GameplayTagBase
{
	GENERATED_UCLASS_BODY()

	/** use current order tags instead */
	UPROPERTY(EditAnywhere, Category=GameplayTag)
	bool UseOrderTags;

	UPROPERTY(EditAnywhere, Category=GameplayTag)
	bool BroadcastResultToController;

	UPROPERTY(Category = Node, EditAnywhere)
	bool InstantExecute;

	UPROPERTY()
	UBehaviorTreeComponent* MyOwnerComp;

	FDelegateHandle OnAbilityEndHandle;

	UPROPERTY()
	AARTAIController* AIController;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

	UPROPERTY()
	UGameplayAbility* ActiveAbility;

	/** cached description */
	UPROPERTY()
	FString CachedDescription;

	UFUNCTION()
	void OnAbilityEnded(const FAbilityEndedData& Data);
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;

	/** describe decorator and cache it */
	virtual void BuildDescription();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif // WITH_EDITOR

protected:

};
