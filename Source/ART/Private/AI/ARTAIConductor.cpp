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
	ListBuffer = GroupList.IsEmpty() ? 0 : ListBuffer++;
	
	UARTAIGroup* Group = NewObject<UARTAIGroup>();
	GroupList.Add(ListBuffer, Group);
	
	return ListBuffer;
}

UARTAIGroup* UARTAIConductor::GetGroup(int32 Key)
{
	return GroupList.Contains(Key) ?  *GroupList.Find(Key) : nullptr;
}

bool UARTAIConductor::TryRemoveGroup(int32 ListKey)
{
	if(!GroupList.Contains(ListKey)) return false;
	UARTAIGroup* Group = GetGroup(ListKey);

	//get agent list
	TArray<AARTCharacterAI*> AgentList;
	Group->GetAgentList(AgentList);
	
	for(AARTCharacterAI* Agent : AgentList)
	{
		UARTCharacterMovementComponent* MoveComp = Agent->FindComponentByClass<UARTCharacterMovementComponent>();

		if(!MoveComp) continue;
		MoveComp->RemoveFromGroup();
	}
	GroupList.Remove(ListKey);
	UE_LOG(LogTemp, Warning, TEXT("Number of Flock: %i"), GroupList.Num());
	return true;
}

bool UARTAIConductor::TryAddAgentToGroup(int32 ListKey, AARTCharacterAI* InAgent)
{
	//do we have this group
	if(!GroupList.Contains(ListKey)) return false;

	//add and check if fail
	UARTAIGroup* Group = GetGroup(ListKey);
	if(!Group->AddAgent(InAgent)) return false;

	UARTCharacterMovementComponent* MoveComp = InAgent->FindComponentByClass<UARTCharacterMovementComponent>();
	if(MoveComp)
	{
		if(MoveComp->GetGroupKey() > -1)
		{
			TryRemoveAgentFromGroup(InAgent);
		}
		//local avoidance
		MoveComp->SetGroupKey(ListKey);
		MoveComp->SetAIConductor(this);
	}

	AARTAIController* Controller = Cast<AARTAIController>(InAgent->GetController());
	if(Controller)
	{
		if(Controller->GetGroupKey() > 0)
		{
			TryRemoveAgentFromGroup(InAgent);
		}
		//group movement
		Controller->SetGroupKey(ListKey);
		Controller->SetAIConductor(this);
	}
	
	return true;
}

bool UARTAIConductor::TryRemoveAgentFromGroup(AARTCharacterAI* InAgent)
{
	if(!InAgent) return false;
	
	UARTCharacterMovementComponent* MoveComp = InAgent->FindComponentByClass<UARTCharacterMovementComponent>();
	if(!MoveComp) return false;

	int32 AgentGroupKey = MoveComp->GetGroupKey();
	if(AgentGroupKey<0) return false;

	if(!GetGroup(AgentGroupKey)->RemoveAgent(InAgent)) return false;
	
	MoveComp->RemoveFromGroup();
	if(GetGroup(AgentGroupKey)->ShouldBeRemoved()) TryRemoveGroup(AgentGroupKey);
	return true;
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
		DrawDebugPoint(World, PathPoints[i].Location, 25.f, FColor::Green, false);
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
