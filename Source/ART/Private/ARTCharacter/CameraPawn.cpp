// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/CameraPawn.h"

#include "GameplayTagResponseTable.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "Framework/ARTGameState.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "AI/Order/ARTSelectComponent.h"
#include "AI/ARTAIConductor.h"
#include "ARTCharacter/ARTPlayerState.h"
#include "ARTCharacter/AI/ARTAIController.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"
#include "Inventory/Component/ARTInventoryComponent_Active.h"

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

void ACameraPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AARTPlayerState* PS = GetPlayerState<AARTPlayerState>();
	if (PS)
	{
		ASC = Cast<UARTAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		InventoryComponent = Cast<UARTInventoryComponent_Active>(PS->GetInventoryComponent());
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		InitializeAbilitySet();
		InitializeAttributes();
		InitializeTagPropertyMap();
		InitializeTagResponseTable();
	}
}

void ACameraPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AARTPlayerState* PS = GetPlayerState<AARTPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		ASC = Cast<UARTAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		InventoryComponent = Cast<UARTInventoryComponent_Active>(PS->GetInventoryComponent());
		
		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		ASC->InitAbilityActorInfo(PS, this);
		InitializeAttributes();
	}
}

void ACameraPawn::InitializeAbilitySet()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !ASC || ASC->CharacterAbilitiesGiven)
	{
		return;
	}
	
	for(TSoftObjectPtr<UARTAbilitySet> Set : AbilitySets)
	{
		UARTAbilitySet* LoadedSet = Set.IsValid() ? Set.Get() : Set.LoadSynchronous();
		if (LoadedSet)
		{
			AbilitySetHandles.Add(LoadedSet->GiveAbilitySetTo(ASC, this));
			ASC->CharacterAbilitiesGiven = true;
		}
	}
}

void ACameraPawn::InitializeAttributes()
{
	if (!ASC)
	{
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."),
			   *FString(__FUNCTION__), *GetName());
		return;
	}
	
	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(
		DefaultAttributes, 1, EffectContext);
	if (NewHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	}
}

void ACameraPawn::InitializeTagPropertyMap()
{
	TagDelegateMap.Initialize(this, ASC);
}

void ACameraPawn::InitializeTagResponseTable()
{
	if (TagReponseTable)
	{
		TagReponseTable->RegisterResponseForEvents(ASC);
	}
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();

	//only server spawn
	if(HasAuthority())
	{
		InitSpawnPlayerTeam();
	}
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

	//PlayerInputComponent->BindAction("Q", IE_Pressed, this, &ACameraPawn::PressQ);
	//PlayerInputComponent->BindAction("Q", IE_Released, this, &ACameraPawn::ReleaseQ);
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

UAbilitySystemComponent* ACameraPawn::GetAbilitySystemComponent() const
{
	return ASC;
}

UARTInventoryComponent* ACameraPawn::GetInventoryComponent() const
{
	return InventoryComponent;
}

void ACameraPawn::InitSpawnPlayerTeam()
{
	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector OffsetPosition(0);
	if(PlayerPawnClass)
	{
		PlayerPawn = GetWorld()->SpawnActor<AARTCharacterAI>(PlayerPawnClass, GetActorLocation() + FVector(0, 0, 0), FRotator(0), Params);
		OffsetPosition = PlayerPawn->GetActorLocation();
	}

	for( auto& PawnClass : AlliesPawnClassList)
	{
		OffsetPosition += FVector(0, -100, 0);
		AARTCharacterAI* AllyPawn = GetWorld()->SpawnActor<AARTCharacterAI>(PawnClass, OffsetPosition, FRotator(0), Params);

		if(UARTAIConductor* AIConductor = AARTGameState::GetAIConductor(this))
		{
			AIConductor->AddAlliesToList(AllyPawn);
		}
	}

	ChangeCurrentUnitInternal(PlayerPawn);
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

AARTCharacterAI* ACameraPawn::BP_GetControlledUnit() const
{
	return PlayerPawn;
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

void ACameraPawn::BP_ChangeCurrentUnit(AARTCharacterAI* Pawn, bool& Success)
{
	if (!Pawn)
	{
		Success = false;
		return;
	}
	Success = ChangeCurrentUnitInternal(Pawn);
}
