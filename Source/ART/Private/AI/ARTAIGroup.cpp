// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ARTAIGroup.h"
#include "DrawDebugHelpers.h"
#include "ARTCharacter/AI/ARTAIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UARTAIGroup::UARTAIGroup()
{
	LeaderID = 0;
	Destination = FVector(FLT_MAX);
}

void UARTAIGroup::Update(float DeltaSeconds)
{
	CalcCentroid();
}

AARTCharacterAI* UARTAIGroup::GetLeader()
{
	return (AgentList.Num() > LeaderID && LeaderID >= 0) ? AgentList[LeaderID] : nullptr;
}

int32 UARTAIGroup::GetLeaderID()
{
	return LeaderID;
}

void UARTAIGroup::SetLeaderID(int32 InLeaderID)
{
	LeaderID = InLeaderID;
}

bool UARTAIGroup::GetAgentList(TArray<AARTCharacterAI*>& OutAgentList)
{
	if(AgentList.Num() == 0) return false;
	
	OutAgentList = AgentList;
	return true;
}

bool UARTAIGroup::AddAgent(AARTCharacterAI* InAgent)
{
	if(AgentList.Contains(InAgent)) return false;
	
	AgentList.Add(InAgent);
	return true;
}

bool UARTAIGroup::RemoveAgent(AARTCharacterAI* InAgent)
{
	if(!AgentList.Contains(InAgent)) return false;

	AgentList.Remove(InAgent);
	InAgent->RemoveFromGroup();
	return true;
}

bool UARTAIGroup::IsEmpty()
{
	return AgentList.Num() == 0? true : false;
}

bool UARTAIGroup::ShouldBeRemoved()
{
	return AgentList.Num() <= 1 ? true : false;
}

FVector UARTAIGroup::GetGroupCentroid()
{
	return Centroid;
}

FVector UARTAIGroup::GetDestination()
{
	return Destination;
}

void UARTAIGroup::SetDestination(FVector& InDestination)
{
	Destination = InDestination;
}

FVector UARTAIGroup::CalcCentroid()
{
	Centroid = FVector(0);
	for(auto& Agent : AgentList)
	{
		Centroid += Agent->GetActorLocation();
	}
	Centroid /= AgentList.Num();

	DrawDebugPoint(GetWorld(), Centroid, 20.f, FColor::Purple);

	return Centroid;
}

void UARTAIGroup::FormFormationAtLocation(FVector FormationPosition, FVector Forward)
{
	//we hard code formation position for now
	float Offset = 100.f;
	Forward.Z = 0;
	ForwardVector = Forward.GetSafeNormal();

	FVector LeftVector = ForwardVector.RotateAngleAxis(-90, FVector::UpVector);
	FVector RightVector = ForwardVector.RotateAngleAxis(90, FVector::UpVector);
	FVector BackVector = ForwardVector.RotateAngleAxis(180, FVector::UpVector);
	
	TArray<FVector> FormationPositions;
	FormationPositions.AddDefaulted(9);
	FormationPositions[4] = FormationPosition;
	FormationPositions[1] = FormationPositions[4] + ForwardVector * Offset;
	FormationPositions[0] = FormationPositions[1] + LeftVector * Offset;
	FormationPositions[2] = FormationPositions[1] + RightVector * Offset;
	FormationPositions[3] = FormationPositions[4] + LeftVector * Offset;
	FormationPositions[5] = FormationPositions[4] + RightVector * Offset;
	FormationPositions[7] = FormationPositions[4] + BackVector * Offset;
	FormationPositions[6] = FormationPositions[7] + LeftVector * Offset;
	FormationPositions[8] = FormationPositions[7] + RightVector * Offset;

	for(auto& Position : FormationPositions)
	{
		DrawDebugPoint(GetWorld(), Position, 20.f, FColor::Blue, false, 2.0f);
	}


	for(int32 i = 0; i < AgentList.Num(); i ++)
	{
		FAIMoveRequest MoveRequest;
		UAIBlueprintHelperLibrary::CreateMoveToProxyObject(
			GetWorld(),
			AgentList[i],
			FormationPositions[i]);
	}
}
