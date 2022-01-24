// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMap.h"


// Sets default values
AInfMap::AInfMap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInfMap::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInfMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

