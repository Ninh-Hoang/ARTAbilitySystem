// Fill out your copyright notice in the Description page of Project Settings.


#include "World/LootBox.h"
#include "Net/UnrealNetwork.h"
#include "Item/Item.h"

// Sets default values
ALootBox::ALootBox()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ALootBox::BeginPlay()
{
	Super::BeginPlay();
}

void ALootBox::SpawnItem()
{
	ServerSpawnItem();
}

void ALootBox::ServerSpawnItem_Implementation()
{
	if (Item)
	{
		FVector Forward = GetActorForwardVector();
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();
		FActorSpawnParameters SpawnParams;
		GetWorld()->SpawnActor<AActor>(Item, SpawnLocation, SpawnRotation, SpawnParams);
	}
}

bool ALootBox::ServerSpawnItem_Validate()
{
	return true;
}
