// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ARTGameState.generated.h"

/**
 * 
 */

class UARTAIConductor;

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	WaveInProgress,

	//no longer spawn bot
	WaitingToComplete,
	WaveComplete,
	GameOver
};

UCLASS()
class ART_API AARTGameState : public AGameState
{
	GENERATED_BODY()

	AARTGameState();
	
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
    EWaveState WaveState;
          	
    UPROPERTY()
    UARTAIConductor* AIConductor;
	
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent)
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	virtual void BeginPlay() override;
	
public:
	//Returns pointer to the AIDirector in the world
	UFUNCTION(BlueprintPure, BlueprintAuthorityOnly, Meta = (WorldContext = "WorldContextObject", CompactNodeTitle = "AIConductor"), Category = "Level|LevelManager")
    static class UARTAIConductor* GetAIConductor(const UObject* WorldContextObject);

	void SetWaveState(EWaveState NewState);
};
