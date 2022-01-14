// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ARTAIConductor.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AI/ARTAIGroup.h"
#include "ARTCharacter/ARTCharacterMovementComponent.h"
#include "ARTCharacter/AI/ARTAIController.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "Kismet/GameplayStatics.h"

UARTAIConductor::UARTAIConductor()
{
	PrimaryComponentTick.bCanEverTick = true;
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

void UARTAIConductor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (const TPair<int32, UARTAIGroup*>& Pair : GroupList)
	{
		Pair.Value->Update(DeltaTime);
	}
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
	TryRemoveAgentFromGroup(AI);
}

void UARTAIConductor::AddLocationToList(FVector InLocation)
{
	MoveLocations.Add(InLocation);
}

TArray<AARTCharacterAI*> UARTAIConductor::GetAgentInGroup(int32 Key)
{
	TArray<AARTCharacterAI*> AgentList;
	if(GroupList.Contains(Key))
	{
		UARTAIGroup* Group = *GroupList.Find(Key);
		Group->GetAgentList(AgentList);
	}

	return AgentList;
}

int32 UARTAIConductor::CreateEmptyGroup()
{
	//reset to 0 for some reason
	ListBuffer++;
	
	UARTAIGroup* Group = NewObject<UARTAIGroup>(this);
	GroupList.Add(ListBuffer, Group);
	
	return ListBuffer;
}

UARTAIGroup* UARTAIConductor::GetGroup(int32 Key)
{
	return GroupList.Contains(Key) ?  *GroupList.Find(Key) : nullptr;
}

bool UARTAIConductor::TryRemoveGroup(int32 ListKey)
{
	UARTAIGroup* Group = GetGroup(ListKey);
	if(!Group) return false;

	//get agent list
	TArray<AARTCharacterAI*> AgentList;
	Group->GetAgentList(AgentList);
	
	for(AARTCharacterAI* Agent : AgentList)
	{
		Agent->RemoveFromGroup();
	}
	
	GroupList.Remove(ListKey);
	return true;
}

bool UARTAIConductor::TryAddAgentToGroup(int32 ListKey, AARTCharacterAI* InAgent)
{
	//do we have this group
	if(!GroupList.Contains(ListKey)) return false;

	//add and check if fail
	UARTAIGroup* Group = GetGroup(ListKey);
	if(!Group->AddAgent(InAgent)) return false;
	
	return true;
}

bool UARTAIConductor::TryRemoveAgentFromGroup(AARTCharacterAI* InAgent)
{
	if(!InAgent) return false;
	
	int32 AgentGroupIndex = InAgent->GetGroupIndex();
	
	if(UARTAIGroup* Group = GetGroup(AgentGroupIndex))
	{
		if(Group->RemoveAgent(InAgent))
		{
			if(Group->ShouldBeRemoved())
			{
				TryRemoveGroup(AgentGroupIndex);
			}
			return true;
		}
	}
	
	return false;
}

int32 UARTAIConductor::GetNumberOfGroup()
{
	return GroupList.Num();
}

int32 UARTAIConductor::GetGroupUnitCount(int32 GroupKey)
{
	return GetAgentInGroup(GroupKey).Num();
}

AARTCharacterAI* UARTAIConductor::GetLeader(int32 GroupKey)
{
	if(!GroupList.Contains(GroupKey)) return nullptr;
	
	return GetGroup(GroupKey)->GetLeader();
}

bool UARTAIConductor::FindPathForGroup(int32 GroupKey, FVector& PathEnd, TArray<FNavPathPoint>& PathLanes)
{
	UARTAIGroup* Group = GetGroup(GroupKey);
	if(!Group) return false;

	AARTCharacterAI* Leader = GetLeader(GroupKey);
	if(!Leader) return false;

	//check if target destination is same same
	if(FVector::Distance(Group->GetDestination(), PathEnd) >= 20.f)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		UNavigationPath* LeaderPath = NavSys->FindPathToLocationSynchronously(GetWorld(), Leader->GetActorLocation(), PathEnd, NULL);
		if(!LeaderPath->IsValid()) return false;

		TArray<FNavPathPoint>PathPoints = LeaderPath->GetPath()->GetPathPoints();

		DrawDebugPath(PathPoints, FColor::Magenta);
	
		const int32 PathPointNum = PathPoints.Num();
	
		if(PathPointNum < 3) return false;

		const UWorld* World = GetWorld();

		//turning map
		TArray<bool> TurnRightArray;
		TArray<FVector> OffsetDirections;
		for(int32 i = 0; i < PathPointNum; i++)
		{
			if(i == 0 ||i == PathPointNum-1) continue;

			FVector A = PathPoints[i-1];
			FVector B = PathPoints[i];
			FVector C = PathPoints[i+1];

			FVector AB = B-A;
			FVector BC = C-B;
			FVector AC = C-A;

			FVector CrossVector = FVector::CrossProduct(AC, AB);
		
			float Angle = CrossVector.Z > 0 ? 90.f : -90.f;
			bool bTurnRight =  CrossVector.Z > 0 ? false : true;
			
			TurnRightArray.Add(bTurnRight);
		
			FVector OffsetDirection = AC.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();
			OffsetDirections.Add(OffsetDirection);
		
			DrawDebugLine(World, PathPoints[i].Location,
				PathPoints[i].Location + OffsetDirection*140.f,
				FColor::Purple,
				false,
				3.f,
				0,
				5.f);
		}

		TArray<TArray<FNavPathPoint>> Paths;
		int32 LaneCount = 7;
		float LaneDistance = 70.f;
		bool FirstTurn = TurnRightArray[0];

		for(int32 l = 0; l < LaneCount; l++)
		{
			TArray<FNavPathPoint> Lane = PathPoints;
			for(int32 i = 1; i < PathPointNum-1; i++)
			{
				//offset Key because we ignore first point
				if(i == PathPointNum -1) continue;
				int32 Key = i-1;
			
				//if have different turn side, inverse offset direction
				bool bNeedInverseOffset = TurnRightArray[Key] == FirstTurn ? false : true;
			
				if(!bNeedInverseOffset)
				{
					Lane[i].Location = Lane[i].Location + l * LaneDistance * OffsetDirections[Key];
				}
				else
				{
					int32 LaneOffSet = LaneCount-l-1;
					Lane[i].Location = Lane[i].Location + LaneOffSet * LaneDistance * OffsetDirections[Key];
				}
			}
			Paths.Add(Lane);
		}

		//Group->Paths = Paths;

		for(auto& Lane : Paths)
		{
			DrawDebugPath(Lane, FColor::Green);
		}
	}
	
	//PathLanes = Group->Paths[FMath::RandRange(0, 6)];
	
	return false;
}

void UARTAIConductor::DrawDebugPathForGroup(int32 GroupKey, FVector PathEnd)
{
	TArray<FNavPathPoint> Temp;
	FindPathForGroup(GroupKey,  PathEnd, Temp);
}

void UARTAIConductor::DrawDebugPath(const TArray<FNavPathPoint> PathPoints, FColor Color)
{
	int32 PathPointNum = PathPoints.Num();
	const UWorld* World = GetWorld();
	
	for(int32 i = 0; i < PathPointNum; i++)
	{
		DrawDebugPoint(World, PathPoints[i].Location, 25.f, FColor::Green, false, 3.f);
		if(i == 0) continue;
		
		DrawDebugLine(World, PathPoints[i-1].Location,
			PathPoints[i].Location,
			Color,
			false,
			3.f,
			0,
			2.f);
	}
}
