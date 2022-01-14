// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacter/ARTPathFollowingComponent.h"

#include "Components/ActorComponent.h"
#include "ARTAIConductor.generated.h"

class AARTCharacterAI;

UCLASS()
class ART_API UARTAIConductor : public UActorComponent
{
	GENERATED_BODY()
	friend  class UARTAIGroup;
public:	
	// Sets default values for this actor's properties
	UARTAIConductor();

	void Activate(bool bNewAutoActivate) override;

	UPROPERTY(BlueprintReadWrite)
	TMap<AActor*, FVector> ActorLocationMap;

protected:

	TArray<AARTCharacterAI*> AlliesList;
	TArray<AARTCharacterAI*> AIList;

	TArray<FVector> MoveLocations;

public:
	//General Allies List
	UFUNCTION(BlueprintPure, Category="AIManager")
	TArray<AARTCharacterAI*> GetAlliesList() const;
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
	void AddAlliesToList(AARTCharacterAI* AI);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	void RemoveAlliesFromList(AARTCharacterAI* AI);
	
	UFUNCTION(BlueprintPure, Category="AIManager")
	TArray<AARTCharacterAI*> GetAIList() const;

	UFUNCTION(BlueprintPure, Category="AIManager")
    TArray<FVector> GetAIMoveToLocation() const;
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
    void AddAIToList(AARTCharacterAI* AI);

	UFUNCTION(BlueprintCallable, Category="AIManager")
    void RemoveAIFromList(AARTCharacterAI* AI);

	UFUNCTION(BlueprintCallable, Category="AIManager")
    void AddLocationToList(FVector Location);

	// Group List
	UPROPERTY()
	TMap<int32, UARTAIGroup*> GroupList;
	int32 ListBuffer;

	UFUNCTION(BlueprintPure, Category="AIManager")
	TArray<AARTCharacterAI*> GetAgentInGroup(int32 Key);
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
	int32 CreateEmptyGroup();
	
	UARTAIGroup* GetGroup(int32 Key);
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryRemoveGroup(int32 ListKey);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryAddAgentToGroup(int32 ListKey, AARTCharacterAI* InAgent);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryRemoveAgentFromGroup(AARTCharacterAI* InAgent);

	UFUNCTION(BlueprintPure, Category="AIManager")
	int32 GetNumberOfGroup();

	UFUNCTION(BlueprintPure, Category="AIManager")
	int32 GetGroupUnitCount(int32 GroupIndex);

	UFUNCTION(BlueprintPure, Category="AIManager")
	AARTCharacterAI* GetLeader(int32 GroupIndex);

	bool FindPathForGroup(int32 GroupIndex, FVector& PathEnd, TArray<FNavPathPoint>& PathLanes);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	void DrawDebugPathForGroup(int32 GroupIndex, FVector PathEnd);

	void DrawDebugPath(const TArray<FNavPathPoint> PathPoints, FColor Color);
};

