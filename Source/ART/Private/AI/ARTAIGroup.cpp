// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ARTAIGroup.h"

UARTAIGroup::UARTAIGroup()
{
	LeaderID = 0;
	Destination = FVector(FLT_MAX);
}

void UARTAIGroup::Update(float DeltaSeconds)
{
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
	if(AgentList.IsEmpty()) return false;
	
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
	return true;
}

bool UARTAIGroup::IsEmpty()
{
	return AgentList.IsEmpty()? true : false;
}

bool UARTAIGroup::ShouldBeRemoved()
{
	return AgentList.Num() <= 1 ? true : false;
}

FVector UARTAIGroup::GetDestination()
{
	return Destination;
}

void UARTAIGroup::SetDestination(FVector& InDestination)
{
	Destination = InDestination;
}
