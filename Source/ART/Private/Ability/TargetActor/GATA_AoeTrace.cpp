// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/TargetActor/GATA_AoeTrace.h"
#include "GameFramework/PlayerController.h"

AGATA_AoeTrace::AGATA_AoeTrace()
{
}

void AGATA_AoeTrace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector WorldLocation;
	FVector WorldDirection;

	MasterPC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	FVector ActorLocation = MasterPC->GetPawn()->GetActorLocation();

	FVector Intersection = FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000,
	                                                    ActorLocation, FVector::UpVector);

	SetActorLocation(Intersection);
}
