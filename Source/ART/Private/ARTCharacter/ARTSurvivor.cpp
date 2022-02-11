// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTSurvivor.h"
#include "ARTCharacter/ARTPlayerState.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "ARTCharacter/ARTCharacterBase.h"
#include <Camera/CameraComponent.h>
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include <Engine/World.h>
#include "Item/InventoryComponent.h"
#include "Item/Item.h"
#include "World/Pickup.h"
#include "ARTCharacter/ARTPlayerState.h"
#include <GameFramework/PlayerState.h>
#include <GameplayEffect.h>
#include <ARTCharacter/ARTPlayerController.h>
#include <Blueprint/UserWidget.h>
#include <Components/WidgetComponent.h>
#include <Widget/ARTFloatingStatusBarWidget.h>
#include "Item/InventorySet.h"

AARTSurvivor::AARTSurvivor(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AzimuthComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Azimuth"));
	AzimuthComponent->SetupAttachment(RootComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(AzimuthComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Item.Weapon.None"));
	//WeaponChangingDelayReplicationTag = FGameplayTag::RequestGameplayTag(
	//	FName("Ability.Weapon.IsChangingDelayReplication"));

	WeaponAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon"));
}

void AARTSurvivor::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AARTPlayerState* PS = GetPlayerState<AARTPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		ASC = Cast<UARTAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		InventoryComponent = Cast<UInventoryComponent>(PS->GetInventoryComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.

		InitializeAttributes();
		
		AddCharacterAbilitiesAndEffects();

		InitializeTagPropertyMap();

		InitializeTagResponseTable();

		AARTPlayerController* PC = Cast<AARTPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		//InitializeFloatingStatusBar();
		// Respawn specific things that won't affect first possession.

		// Forcibly set the DeadTag count to 0
		//AbilitySystemComponent->SetTagMapCount(DeadTag, 0);

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		if (ASC->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
			//SetShield(GetMaxShield());
			//SetHealth(GetMaxHealth());
			//SetStamina(GetMaxStamina());
		}

		// Remove Dead tag
		ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(DeadTag));

		InitializeFloatingStatusBar();

		// If player is host on listen server, the floating status bar would have been created for them from BeginPlay before player possession, hide it
		if (IsLocallyControlled() && IsPlayerControlled() && UIFloatingStatusBarComponent && UIFloatingStatusBar)
		{
			UIFloatingStatusBarComponent->SetVisibility(false, true);
		}
	}
}


void AARTSurvivor::BeginPlay()
{
	Super::BeginPlay();

	InitializeFloatingStatusBar();
}

void AARTSurvivor::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AARTPlayerState* PS = GetPlayerState<AARTPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		ASC = Cast<UARTAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Set the inventory for clients. Server does this in PossessedBy.
		InventoryComponent = Cast<UInventoryComponent>(PS->GetInventoryComponent());

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		ASC->InitAbilityActorInfo(PS, this);

		// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
		BindASCInput();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.

		InitializeAttributes();

		AARTPlayerController* PC = Cast<AARTPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
		//InitializeFloatingStatusBar();


		// Respawn specific things that won't affect first possession.

		// Forcibly set the DeadTag count to 0
		//AbilitySystemComponent->SetTagMapCount(DeadTag, 0);
		if (ASC->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina/Shield to their max. This is only for *Respawn*. It will be set (replicated) by the
			// Server, but we call it here just to be a little more responsive.
			//SetHealth(GetMaxHealth());
			//SetShield(GetMaxShield());
			//SetStamina(GetMaxStamina());
		}
	}

	// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
	InitializeFloatingStatusBar();
}

//DEAD STUFFS
void AARTSurvivor::Die()
{
	Super::Die();
}

//Player unique UI STUFFS
UARTFloatingStatusBarWidget* AARTSurvivor::GetFloatingStatusBar()
{
	return UIFloatingStatusBar;
}

void AARTSurvivor::InitializeFloatingStatusBar()
{
	// Only create once
	if (UIFloatingStatusBar || !IsValid(ASC))
	{
		return;
	}

	// Don't create for locally controlled player. We could add a game setting to toggle this later.
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		return;
	}

	// Need a valid PlayerState
	if (!GetPlayerState())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	AARTPlayerController* PC = Cast<AARTPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UARTFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);

			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				//UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				//UIFloatingStatusBar->SetEnergyPercentage(GetEnergy() / GetMaxEnergy());
				//UIFloatingStatusBar->SetShieldPercentage(GetShield() / GetMaxShield());
				UIFloatingStatusBar->OwningCharacter = this;
				UIFloatingStatusBar->SetCharacterName(CharacterName);
			}
		}
	}
}

//EQUIPMENT LIST
void AARTSurvivor::OnRep_Equipment()
{
	if (GetLocalRole() == ROLE_AutonomousProxy && Equipment.Weapons.Num() > 0 && !CurrentWeapon)
	{
		// Since we don't replicate the CurrentWeapon to the owning client, this is a way to ask the Server to sync
		// the CurrentWeapon after it's been spawned via replication from the Server.
		// The weapon spawning is replicated but the variable CurrentWeapon is not on the owning client.
		ServerSyncCurrentWeapon();
	}
}

//WEAPON STUFFS
AWeapon* AARTSurvivor::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void AARTSurvivor::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void AARTSurvivor::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
{
	if (NewWeapon == LastWeapon)
	{
		return;
	}
	// Cancel active weapon abilities
	if (ASC)
	{
		FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(WeaponAbilityTag);
		ASC->CancelAbilities(&AbilityTagsToCancel);
	}

	if (LastWeapon)
	{
		UnEquipWeapon(LastWeapon);
	}

	if (NewWeapon)
	{
		if (ASC)
		{
			// Clear out potential NoWeaponTag
			ASC->RemoveLooseGameplayTag(CurrentWeaponTag);
		}

		// Weapons coming from OnRep_CurrentWeapon won't have the owner set
		CurrentWeapon = NewWeapon;
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->EquipWeapon();
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		ClientSyncCurrentWeapon(CurrentWeapon);

		AARTPlayerController* PC = GetController<AARTPlayerController>();
		if (PC && PC->IsLocalController())
		{
			PC->SetHUDReticle(CurrentWeapon->GetPrimaryHUDReticleClass());
		}

		if (ASC)
		{
			ASC->AddLooseGameplayTag(CurrentWeaponTag);
		}

		UAnimMontage* EquipMontage = CurrentWeapon->GetEquipWeaponMontage();

		if (EquipMontage && GetMesh())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(EquipMontage);
		}
	}
	else
	{
		if (CurrentWeapon)
		{
			UnEquipCurrentWeapon();
		}
	}
}

void AARTSurvivor::ServerSyncCurrentWeapon_Implementation()
{
	ClientSyncCurrentWeapon(CurrentWeapon);
}

bool AARTSurvivor::ServerSyncCurrentWeapon_Validate()
{
	return true;
}

void AARTSurvivor::ClientSyncCurrentWeapon_Implementation(AWeapon* InWeapon)
{
	AWeapon* LastWeapon = CurrentWeapon;
	CurrentWeapon = InWeapon;
	OnRep_CurrentWeapon(LastWeapon);
}

bool AARTSurvivor::ClientSyncCurrentWeapon_Validate(AWeapon* InWeapon)
{
	return true;
}

void AARTSurvivor::EquipWeapon(AWeapon* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
		bChangedWeaponLocally = true;
	}
}

void AARTSurvivor::ServerEquipWeapon_Implementation(AWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool AARTSurvivor::ServerEquipWeapon_Validate(AWeapon* NewWeapon)
{
	return true;
}

void AARTSurvivor::UnEquipWeapon(AWeapon* WeaponToUnEquip)
{
	WeaponToUnEquip->UnEquipWeapon();
}

void AARTSurvivor::UnEquipCurrentWeapon()
{
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		ASC->AddLooseGameplayTag(CurrentWeaponTag);
	}

	UnEquipWeapon(CurrentWeapon);
	CurrentWeapon = nullptr;
}

AWeapon* AARTSurvivor::AddWeaponToEquipment(TSubclassOf<AWeapon> WeaponClass)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerAddWeaponToEquipment(WeaponClass);
	}

	if (WeaponClass)
	{
		AWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<AWeapon>(WeaponClass,
		                                                             FTransform::Identity, this, this,
		                                                             ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		//setup Weapon before finish spawning
		//NewWeapon->bSpawnWithCollision = false;

		NewWeapon->FinishSpawning(FTransform::Identity);
		NewWeapon->Equip(this);

		Equipment.Weapons.Add(NewWeapon);
		NewWeapon->SetOwningCharacter(this);

		if (GetLocalRole() < ROLE_Authority)
		{
			return NewWeapon;
		}

		NewWeapon->AddAbilities();

		return NewWeapon;
	}

	return nullptr;
}

void AARTSurvivor::ServerAddWeaponToEquipment_Implementation(TSubclassOf<AWeapon> WeaponClass)
{
	AddWeaponToEquipment(WeaponClass);
}

bool AARTSurvivor::ServerAddWeaponToEquipment_Validate(TSubclassOf<AWeapon> WeaponClass)
{
	return true;
}

bool AARTSurvivor::DoesWeaponExistInInventory(AWeapon* InWeapon)
{
	//UE_LOG(LogTemp, Log, TEXT("%s InWeapon class %s"), *FString(__FUNCTION__), *InWeapon->GetClass()->GetName());

	for (AWeapon* Weapon : Equipment.Weapons)
	{
		if (Weapon && InWeapon && Weapon == InWeapon)
		{
			return true;
		}
	}

	return false;
}

//EQUIPMENT STUFFS

TArray<AEquipment*> AARTSurvivor::GetEquipmentArray() const
{
	return Equipment.Equipments;
}

void AARTSurvivor::EquipEquipment(AEquipment* NewEquipment)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipEquipment(NewEquipment);
	}
	else
	{
		if (NewEquipment)
		{
			NewEquipment->SetOwningCharacter(this);
			NewEquipment->Equip(this);

			if (ASC)
			{
				ASC->AddLooseGameplayTag(NewEquipment->EquipmentTag);
			}

			UAnimMontage* EquipMontage = NewEquipment->GetEquipMontage();

			if (EquipMontage && GetMesh())
			{
				GetMesh()->GetAnimInstance()->Montage_Play(EquipMontage);
			}
		}
	}
}

void AARTSurvivor::ServerEquipEquipment_Implementation(AEquipment* NewEquipment)
{
	EquipEquipment(NewEquipment);
}

bool AARTSurvivor::ServerEquipEquipment_Validate(AEquipment* NewEquipment)
{
	return true;
}

AEquipment* AARTSurvivor::AddEquipmentToEquipmentList(TSubclassOf<AEquipment> EquipmentClass)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerAddEquipmentToEquipmentList(EquipmentClass);
	}

	if (EquipmentClass)
	{
		AEquipment* NewEquipment = GetWorld()->SpawnActorDeferred<AEquipment>(EquipmentClass,
		                                                                      FTransform::Identity, this, this,
		                                                                      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		//setup Equipment before finish spawning
		//NewEquipment->bSpawnWithCollision = false;

		NewEquipment->FinishSpawning(FTransform::Identity);
		NewEquipment->Equip(this);

		Equipment.Equipments.Add(NewEquipment);
		NewEquipment->SetOwningCharacter(this);

		if (GetLocalRole() < ROLE_Authority)
		{
			return NewEquipment;
		}

		NewEquipment->AddAbilities();

		return NewEquipment;
	}

	return nullptr;
}

bool AARTSurvivor::DoesEquipmentExistInInventory(AEquipment* InEquipment)
{
	//UE_LOG(LogTemp, Log, TEXT("%s InWeapon class %s"), *FString(__FUNCTION__), *InWeapon->GetClass()->GetName());

	for (AEquipment* IndexEquipment : Equipment.Equipments)
	{
		if (IndexEquipment && InEquipment && IndexEquipment == InEquipment)
		{
			return true;
		}
	}

	return false;
}

void AARTSurvivor::ServerAddEquipmentToEquipmentList_Implementation(TSubclassOf<AEquipment> EquipmentClass)
{
	AddEquipmentToEquipmentList(EquipmentClass);
}

bool AARTSurvivor::ServerAddEquipmentToEquipmentList_Validate(TSubclassOf<AEquipment> EquipmentClass)
{
	return true;
}

//ITEM USING / INVENTORY
void AARTSurvivor::UseItem(UItem* Item)
{
	//if is client, run on server
	if (!HasAuthority() && Item)
	{
		ServerUseItem(Item);
	}

	//if is server, check if the wanted item is in inventory, if not return
	if (HasAuthority())
	{
		if (InventoryComponent && !InventoryComponent->FindItem(Item))
		{
			return;
		}
	}

	if (Item)
	{
		Item->OnUse(this);
		Item->Use(this);
	}
}

void AARTSurvivor::ServerUseItem_Implementation(UItem* Item)
{
	UseItem(Item);
}

bool AARTSurvivor::ServerUseItem_Validate(UItem* Item)
{
	return true;
}

void AARTSurvivor::DropItem(UItem* Item, int32 Quantity)
{
	if (Quantity <= 0)
	{
		return;
	}
	//if is client, run on server 
	if (!HasAuthority())
	{
		ServerDropItem(Item, Quantity);
		return;
	}

	if (HasAuthority())
	{
		const int32 ItemQuantity = Item->GetQuantity();
		const int32 DroppedQuantity = InventoryComponent->ConsumeItem(Item, Quantity);

		//spawn pickup
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = GetActorLocation();
		//SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		ensure(PickupClass);

		APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
		Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
	}
}

void AARTSurvivor::ServerDropItem_Implementation(UItem* Item, int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool AARTSurvivor::ServerDropItem_Validate(UItem* Item, int32 Quantity)
{
	return true;
}

void AARTSurvivor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AARTSurvivor, Equipment);
	DOREPLIFETIME_CONDITION(AARTSurvivor, CurrentWeapon, COND_SimulatedOnly);
}
