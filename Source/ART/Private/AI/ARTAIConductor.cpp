// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ARTAIConductor.h"

#include "ARTCharacter/ARTCharacterMovementComponent.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "Kismet/GameplayStatics.h"

UARTAIConductor::UARTAIConductor()
{
	SetAutoActivate(true);
}

void UARTAIConductor::Activate(bool bNewAutoActivate)
{
	Super::Activate();
	
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),AARTCharacterAI::StaticClass(),AllActors);
	
	TArray<AARTCharacterAI*> AIActors;

	for(auto& Actor : AllActors)
	{
		AIActors.Add(Cast<AARTCharacterAI>(Actor));
	}
	
	AIList.Append(AIActors);

	UE_LOG(LogTemp, Warning, TEXT("Number of AI at Start: %i"), AIList.Num());
}

TArray<AARTCharacterAI*> UARTAIConductor::GetAlliesList() const
{
	return AlliesList;
}

void UARTAIConductor::AddAlliesToList(AARTCharacterAI* AI)
{
	if(AI && !AI->IsPendingKillPending())
	{
		AlliesList.Add(AI);
	}
}

void UARTAIConductor::RemoveAlliesFromList(AARTCharacterAI* AI)
{
	AlliesList.RemoveSingle(AI);
}

TArray<AARTCharacterAI*> UARTAIConductor::GetAIList() const
{
	return AIList;
}

TArray<FVector> UARTAIConductor::GetAIMoveToLocation()const
{
	return MoveLocations;
}

void UARTAIConductor::AddAIToList(AARTCharacterAI* AI)
{
	if(AI && !AI->IsPendingKillPending())
	{
		AIList.Add(AI);
	}
}

void UARTAIConductor::RemoveAIFromList(AARTCharacterAI* AI)
{
	AIList.RemoveSingle(AI);
	ActorLocationMap.Remove(AI);
	AlliesList.Remove(AI);
	TryRemoveBoidFromList(AI);
}

void UARTAIConductor::AddLocationToList(FVector InLocation)
{
	MoveLocations.Add(InLocation);
}


TArray<AARTCharacterAI*> UARTAIConductor::GetBoidList(int32 Key) const
{
	if(BoidMap.Contains(Key)) return BoidMap.Find(Key)->BoidList;
	TArray<AARTCharacterAI*> Empty;
	return Empty;
}

int32 UARTAIConductor::CreateFlock()
{
	if(BoidMap.IsEmpty())
	{
		BoidMap.Emplace(1);
		ListBuffer = 1;
		return 1;
	}

	ListBuffer++;
	BoidMap.Emplace(ListBuffer);
	return ListBuffer;
}

bool UARTAIConductor::TryRemoveBoidList(int32 ListKey)
{
	if(!BoidMap.Contains(ListKey)) return false;
	FFlock* Flock = BoidMap.Find(ListKey);
	for(AARTCharacterAI* Boid : Flock->BoidList)
	{
		UARTCharacterMovementComponent* MoveComp = Boid->FindComponentByClass<UARTCharacterMovementComponent>();

		if(!MoveComp) continue;
		MoveComp->RemoveFromGroup();
	}
	BoidMap.Remove(ListKey);
	UE_LOG(LogTemp, Warning, TEXT("Number of Flock: %i"), BoidMap.Num());
	return true;
}

bool UARTAIConductor::TryAddBoidToList(int32 ListKey, AARTCharacterAI* InBoid)
{
	if(!BoidMap.Contains(ListKey)) return false;
	if(BoidMap.Find(ListKey)->BoidList.Contains(InBoid)) return false;
	
	BoidMap.Find(ListKey)->BoidList.Add(InBoid);

	UARTCharacterMovementComponent* MoveComp = InBoid->FindComponentByClass<UARTCharacterMovementComponent>();
	if(MoveComp)
	{
		if(MoveComp->GetBoidGroupKey() > 0)
		{
			TryRemoveBoidFromList(InBoid);
		}
		MoveComp->SetBoidGroup(ListKey);
		MoveComp->SetAIConductor(this);
	}
	return true;
}

bool UARTAIConductor::TryRemoveBoidFromList(AARTCharacterAI* InBoid)
{
	if(!InBoid) return false;
	
	UARTCharacterMovementComponent* MoveComp = InBoid->FindComponentByClass<UARTCharacterMovementComponent>();
	if(!MoveComp) return false;

	int32 BoidGroupKey = MoveComp->GetBoidGroupKey();
	if(BoidGroupKey<1) return false;

	BoidMap.Find(BoidGroupKey)->BoidList.Remove(InBoid);
	MoveComp->RemoveFromGroup();

	if(BoidMap.Find(BoidGroupKey)->BoidList.Num() < 2) TryRemoveBoidList(BoidGroupKey);
	return true;
}
