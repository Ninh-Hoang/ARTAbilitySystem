// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WeaponActor.h"
#include "Components/MeshComponent.h"
#include "ART/ART.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapon"),
	DebugWeaponDrawing,
	TEXT("Draw Debug For Weapon"),
	ECVF_Cheat);

// Sets default values
AWeaponActor::AWeaponActor()
{
	RateOfFire = 600;

	SetReplicates(true);

	ThreatValue = 70;

	FireRotationSpeed = 50;

	FirstShotDelay = 0.0f;

	Range = 10000;
}

// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
	TimeBetweenShot = 60 / RateOfFire;
}

// Called every frame

void AWeaponActor::Initialize(UMeshComponent* MeshComponentToSet)
{
	MeshComponent = MeshComponentToSet;
}
