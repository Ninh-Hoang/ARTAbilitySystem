// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "ARTAIConductor.generated.h"

class AARTCharacterAI;

USTRUCT()
struct FFlock
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<AARTCharacterAI*> BoidList;
};

UCLASS()
class ART_API UARTAIConductor : public UActorComponent
{
	GENERATED_BODY()
	
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
	//AlliesList
	UFUNCTION(BlueprintPure, Category="AIManager")
	TArray<AARTCharacterAI*> GetAlliesList() const;
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
	void AddAlliesToList(AARTCharacterAI* AI);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	void RemoveAlliesFromList(AARTCharacterAI* AI);
	
	// AI LIST
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
	
	TMap<int32, FFlock> BoidMap;
	int32 ListBuffer;

	UFUNCTION(BlueprintPure, Category="AIManager")
	TArray<AARTCharacterAI*> GetBoidList(int32 Key) const;
	
	UFUNCTION(BlueprintCallable, Category="AIManager")
	int32 CreateFlock();

	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryRemoveBoidList(int32 ListKey);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryAddBoidToList(int32 ListKey, AARTCharacterAI* InBoid);

	UFUNCTION(BlueprintCallable, Category="AIManager")
	bool TryRemoveBoidFromList(AARTCharacterAI* InBoid);
};

