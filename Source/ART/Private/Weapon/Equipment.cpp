// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipment.h"
#include "Net/UnrealNetwork.h"
#include <Components/SkeletalMeshComponent.h>
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTSurvivor.h"

#include "Ability/ARTAbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"

#include <Ability/TargetActor/GATA_AoeTrace.h>
#include <Ability/TargetActor/GATA_MeleeWeaponTrace.h>
#include <Ability/TargetActor/GATA_LineTrace.h>
#include <Ability/TargetActor/GATA_SphereTrace.h>

// Sets default values
AEquipment::AEquipment()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Azimuth"));

	SetRootComponent(SkeletalMeshComponent);

	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EquipmentSlot = EEquipmentSlot::ES_Shield;

	StatusText = DefaultStatusText;
}

AEquipment::~AEquipment()
{
	if (IsValid(LineTraceTargetActor) && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		LineTraceTargetActor->Destroy();
	}
	if (IsValid(SphereTargetActor) && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		SphereTargetActor->Destroy();
	}
	if (IsValid(MeleeTargetActor) && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		MeleeTargetActor->Destroy();
	}
	if (IsValid(SphereTargetActor) && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		SphereTargetActor->Destroy();
	}
}

UAbilitySystemComponent* AEquipment::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

class AGATA_LineTrace* AEquipment::GetLineTraceTargetActor()
{
	if (LineTraceTargetActor)
	{
		return LineTraceTargetActor;
	}

	LineTraceTargetActor = GetWorld()->SpawnActor<AGATA_LineTrace>();
	LineTraceTargetActor->SetOwner(this);
	return LineTraceTargetActor;
}

class AGATA_SphereTrace* AEquipment::GetSphereTraceTargetActor()
{
	if (SphereTargetActor)
	{
		return SphereTargetActor;
	}

	SphereTargetActor = GetWorld()->SpawnActor<AGATA_SphereTrace>();
	SphereTargetActor->SetOwner(this);
	return SphereTargetActor;
}

class AGATA_MeleeWeaponTrace* AEquipment::GetMeleeTraceTargetActor()
{
	if (MeleeTargetActor)
	{
		return MeleeTargetActor;
	}

	MeleeTargetActor = GetWorld()->SpawnActor<AGATA_MeleeWeaponTrace>();
	MeleeTargetActor->SetOwner(this);
	return MeleeTargetActor;
}

class AGATA_AoeTrace* AEquipment::GetAoeTraceTargetActor()
{
	if (AoeTargetActor)
	{
		return AoeTargetActor;
	}

	AoeTargetActor = GetWorld()->SpawnActor<AGATA_AoeTrace>();
	AoeTargetActor->SetOwner(this);
	return AoeTargetActor;
}

// Called when the game starts or when spawned
void AEquipment::BeginPlay()
{
	Super::BeginPlay();
	ResetEquipment();
}

//EQUIPMENT STUFFS
void AEquipment::Equip(AARTSurvivor* CharacterToEquip)
{
	//stupid conversion from enum to FName, holy shit
	/*const TEnumAsByte<EEquipmentSlot> Enum = EquipmentSlot;
	FString Enum = UEnum::GetValueAsString(Enum.GetValue());
	FName Socket = FName(*Enum);*/

	FText SocketText = UEnum::GetDisplayValueAsText(EquipmentSlot);
	FName SocketName = FName(*SocketText.ToString());

	SetOwner(CharacterToEquip);
	AttachToComponent(CharacterToEquip->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

	OwningCharacter = CharacterToEquip;
}

void AEquipment::UnEquip()
{
}

void AEquipment::ResetEquipment()
{
	StatusText = DefaultStatusText;
}

FText AEquipment::GetDefaultStatusText() const
{
	return DefaultStatusText;
}

//ABILITIES STUFFS
void AEquipment::SetOwningCharacter(AARTCharacterBase* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		AbilitySystemComponent = Cast<UARTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
	}
}

void AEquipment::AddAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s Role: %s ASC is null"), *FString(__FUNCTION__), *GetName(),
		       GET_ACTOR_ROLE_STRING(OwningCharacter));
		return;
	}

	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<UARTGameplayAbility>& Ability : Abilities)
	{
		FGameplayAbilitySpecHandle GrantedHandle = ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID),
			                     static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));

		AbilitySpecHandles.Add(GrantedHandle);
	}
}

void AEquipment::RemoveAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UARTAbilitySystemComponent* ASC = Cast<UARTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}

	// Remove abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		ASC->ClearAbility(SpecHandle);
	}
}

int32 AEquipment::GetAbilityLevel(EARTAbilityInputID AbilityID)
{
	// All abilities for now are level 1
	return 1;
}

USkeletalMeshComponent* AEquipment::GetEquipmentMesh()
{
	return SkeletalMeshComponent;
}

UAnimMontage* AEquipment::GetEquipMontage() const
{
	return EquipMontage;
}

//replication
void AEquipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AEquipment, OwningCharacter, COND_OwnerOnly);
}
