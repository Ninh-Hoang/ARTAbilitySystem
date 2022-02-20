// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Component/ARTInventoryComponent_Storage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ART/ART.h"

UARTInventoryComponent_Storage::UARTInventoryComponent_Storage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StorageSlotCount = 12;
}


void UARTInventoryComponent_Storage::InitializeComponent()
{
	Super::InitializeComponent();

	UpdateStorageInventorySlots();
}

void UARTInventoryComponent_Storage::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(GetOwner()))
		{
			if (ASC->HasAttributeSetForAttribute(InventorySizeAttribute))
			{
				StorageSlotCount = FMath::TruncToInt(ASC->GetNumericAttributeBase(InventorySizeAttribute));
				ASC->GetGameplayAttributeValueChangeDelegate(InventorySizeAttribute).AddUObject(this, &UARTInventoryComponent_Storage::OnInventorySizeAttributeChange);
				UpdateStorageInventorySlots();
			}
		}		
	}	
}


void UARTInventoryComponent_Storage::UpdateStorageInventorySlots()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
		
	//First, Count how many Storage slots we have
	TArray<FARTItemSlotReference> Slots;
	Query_GetAllStorageSlots(Slots);

	const int32 SlotCount = Slots.Num();

	const int32 StorageSlotsToCreate = StorageSlotCount - SlotCount;

	if (StorageSlotsToCreate > 0)
	{
		for (int32 i = 0; i < StorageSlotsToCreate; i++)
		{
			CreateInventorySlot(InvStorageSlotTag.GetTag().GetSingleTagContainer(), nullptr);
		}
	}
	else if(StorageSlotsToCreate < 0)
	{
		//remove the bottom slots
		for (int i = 0; i < FMath::Abs(StorageSlotsToCreate); i++)
		{
			const int32 SlotToRemove = SlotCount - 1 - i;
			RemoveInventorySlot(Slots[SlotToRemove]);
		}
	}
	
}

void UARTInventoryComponent_Storage::OnInventorySizeAttributeChange(const FOnAttributeChangeData& ChangeData)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	StorageSlotCount = FMath::TruncToInt(ChangeData.NewValue);
	UpdateStorageInventorySlots();
}

bool UARTInventoryComponent_Storage::Query_GetAllStorageSlots(TArray<FARTItemSlotReference>& OutSlotRefs)
{
	return Query_GetAllSlots(FARTItemQuery::QuerySlotMatchingTag(InvStorageSlotTag), OutSlotRefs);
}
