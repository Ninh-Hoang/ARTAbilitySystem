// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Item/ARTItemStackWorldObject.h"

#include "Engine/ActorChannel.h"
#include "Inventory/ARTItemStack.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AARTItemStackWorldObject::AARTItemStackWorldObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Default Mesh"));
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;	

	RootComponent = StaticMeshComponent;
	
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

	InventoryStack = nullptr;
}

void AARTItemStackWorldObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	
	DOREPLIFETIME_CONDITION_NOTIFY(AARTItemStackWorldObject, InventoryStack, COND_None, REPNOTIFY_Always);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AARTItemStackWorldObject::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = false;

	if (IsValid(InventoryStack))
	{
		WroteSomething |= Channel->ReplicateSubobject(InventoryStack, *Bunch, *RepFlags);
		WroteSomething |= InventoryStack->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	WroteSomething |= Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	return WroteSomething;
}

// Called when the game starts or when spawned
void AARTItemStackWorldObject::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InventoryStack))
	{
		SetInventoryStack(InventoryStack);
	}
	
}

// Called every frame
void AARTItemStackWorldObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AARTItemStackWorldObject::SetInventoryStack_Implementation(UARTItemStack* InInventoryStack)
{
	InventoryStack = InInventoryStack;

	if (IsValid(InventoryStack)
		&& InventoryStack->HasValidItemData()
		&& IsValid(InventoryStack->GetItemDefinition().GetDefaultObject()->WorldItemModel))
	{
		StaticMeshComponent->SetStaticMesh(InventoryStack->GetItemDefinition().GetDefaultObject()->WorldItemModel);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		if (IsValid(InventoryStack))
		{
			UARTItemStack::TransferStackOwnership(InventoryStack, this);
		}
		this->ForceNetUpdate();
	}
}

UARTItemStack* AARTItemStackWorldObject::GetInventoryStack()
{
	return InventoryStack;
}

void AARTItemStackWorldObject::OnRep_InventoryStack()
{
	SetInventoryStack(InventoryStack);
}

/*
void AARTItemStackWorldObject::OnInteract_Implementation(AActor* Interactor, bool bLongHold)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (UARTInventoryComponent* Inventory = UARTItemBPFunctionLibrary::GetInventoryComponent(Interactor))
	{
		if (Inventory->LootItem(InventoryStack))
		{
			
			SetLifeSpan(0.1);
		}
	}
}
*/
