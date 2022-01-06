// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Item.h"
#include "Net/UnrealNetwork.h"
#include "item/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItem, Quantity);
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

#if WITH_EDITOR
void UItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 0, bStackable ? MaxStackSize : 1);
	}
	else if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, bStackable))
	{
		if (!bStackable)
		{
			MaxStackSize = 1;
			Quantity = 1;
		}
	}
}
#endif

UItem::UItem()
{
	bStackable = true;
	Quantity = 1;
	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;

	MaxStackSize = 2;
	RepKey = 0;
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(AARTSurvivor* Character)
{
}

void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bStackable ? MaxStackSize : 1);
		//UE_LOG(LogTemp, Warning, TEXT("%i"), NewQuantity);
		MarkDirtyForReplication();
	}
}

void UItem::AddedToInventory(UInventoryComponent* Inventory)
{
}

void UItem::OnRep_Quantity()
{
	OnItemModified.Broadcast();
}

void UItem::MarkDirtyForReplication()
{
	// Mark this object for replication
	++RepKey;

	// Mark the array for replication
	if (OwningInventory)
	{
		++OwningInventory->ReplicatedItemsKey;
	}
}

#undef LOCTEXT_NAMESPACE
