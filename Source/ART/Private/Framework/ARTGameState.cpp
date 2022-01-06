// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ARTGameState.h"
#include "Net/UnrealNetwork.h"
#include "AI/ARTAIConductor.h"

AARTGameState::AARTGameState()
{
	AIConductor = CreateDefaultSubobject<UARTAIConductor>(TEXT("AIMananger"));
}

void AARTGameState::BeginPlay()
{
	Super::BeginPlay();
}

UARTAIConductor* AARTGameState::GetAIConductor(const UObject* WorldContextObject)
{
	if(GEngine) {
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
		AARTGameState * GameState = World->GetGameState<AARTGameState>();
		if(GameState && GameState->AIConductor) return GameState->AIConductor;
	}
	return nullptr;
}

void AARTGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void AARTGameState::SetWaveState(EWaveState NewState)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;

		OnRep_WaveState(OldState);
	}
}

void AARTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AARTGameState, WaveState)
}
