// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "UObject/NoExportTypes.h"
#include "ARTAIGroup.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAIGroup : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	int32 LeaderID;

	UPROPERTY()
	TArray<AARTCharacterAI*> AgentList;

	UPROPERTY()
	FVector Destination;
	
	TArray<TArray<FNavPathPoint>> Paths;
	
public:
	UARTAIGroup();
	
	void Update(float DeltaSeconds);
	
	AARTCharacterAI* GetLeader();

	UFUNCTION(BlueprintPure)
	int32 GetLeaderID();

	UFUNCTION(BlueprintCallable)
	void SetLeaderID(int32 InLeaderID);

	bool GetAgentList(TArray<AARTCharacterAI*>& OutAgentList);

	bool AddAgent(AARTCharacterAI* InAgent);
	bool RemoveAgent(AARTCharacterAI* InAgent);

	bool IsEmpty();
	bool ShouldBeRemoved();

	FVector GetDestination();
	void SetDestination(FVector& InDestination);

	
	//CalculatePathLane();
};
