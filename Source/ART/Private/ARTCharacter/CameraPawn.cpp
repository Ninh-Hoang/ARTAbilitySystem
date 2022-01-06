// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/CameraPawn.h"

#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "AI/Order/ARTSelectComponent.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AzimuthComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Azimuth"));
	AzimuthComponent->SetupAttachment(RootComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(AzimuthComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MoveComp"));

	DummyInputComp = CreateDefaultSubobject<UInputComponent>(TEXT("DummyInputComp"));
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	InitSpawnPlayerTeam();
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACameraPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACameraPawn::MoveRight);

	PlayerInputComponent->BindAction("UnitA", IE_Pressed, this, &ACameraPawn::ChangeToUnitA);
	PlayerInputComponent->BindAction("UnitB", IE_Pressed, this, &ACameraPawn::ChangeToUnitB);

	//PlayerInputComponent->BindAction("Q", IE_Pressed, this, &ACameraPawn::PressQ);
	//PlayerInputComponent->BindAction("Q", IE_Released, this, &ACameraPawn::ReleaseQ);
}

void ACameraPawn::InitSpawnPlayerTeam()
{
	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UnitA = GetWorld()->SpawnActor<AARTCharacterAI>(UnitClassA, GetActorLocation() + FVector(0, 100, 0), FRotator(0),
	                                                Params);
	UnitB = GetWorld()->SpawnActor<AARTCharacterAI>(UnitClassA, GetActorLocation() + FVector(0, -100, 0), FRotator(0),
	                                                Params);

	ChangeCurrentUnitInternal(UnitA);
}

bool ACameraPawn::ChangeCurrentUnitInternal(AARTCharacterAI* Unit)
{
	//same unit
	if (SelectedUnits.Num() == 1 && SelectedUnits[0] == Unit) return false;

	bool AlreadySelected = false;

	//iterate and deselect
	for (int32 i = 0; i < SelectedUnits.Num(); i++)
	{
		if (SelectedUnits[i] == Unit)
		{
			AlreadySelected = true;
			continue;
		}
		DeSelectUnit(SelectedUnits[i]);
	}

	//select unit if they are not currently in group
	if (!AlreadySelected) SelectUnit(Unit);
	return true;
}

void ACameraPawn::MoveForward(float AxisValue)
{
	FRotator Rot = FRotator(0, GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(Rot).GetScaledAxis(EAxis::X), AxisValue);
}

void ACameraPawn::MoveRight(float AxisValue)
{
	FRotator Rot = FRotator(0, GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(Rot).GetScaledAxis(EAxis::Y), AxisValue);
}

bool ACameraPawn::SelectUnit(AARTCharacterAI* Unit)
{
	if (!Unit) return false;

	UARTSelectComponent* SelectComp = Unit->FindComponentByClass<UARTSelectComponent>();
	if (!SelectComp) return false;

	SelectedUnits.Add(Unit);

	return SelectComp->SetSelected(true);
}

bool ACameraPawn::DeSelectUnit(AARTCharacterAI* Unit)
{
	if (!Unit) return false;

	UARTSelectComponent* SelectComp = Unit->FindComponentByClass<UARTSelectComponent>();
	if (!SelectComp) return false;

	SelectedUnits.RemoveSingle(Unit);

	return SelectComp->SetSelected(false);
}

void ACameraPawn::ChangeToUnitA()
{
	ChangeCurrentUnitInternal(UnitA);
}

void ACameraPawn::ChangeToUnitB()
{
	ChangeCurrentUnitInternal(UnitB);
}

void ACameraPawn::PressQ()
{
}

void ACameraPawn::ReleaseQ()
{
}

void ACameraPawn::BP_ChangeCurrentUnit(int32 UnitIndex, bool& Success)
{
	switch (UnitIndex)
	{
	case 1:
		Success = ChangeCurrentUnitInternal(UnitA);
		break;
	case 2:
		Success = ChangeCurrentUnitInternal(UnitB);
		break;
	}
}
