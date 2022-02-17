// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Item.h"
#include "item/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "Item"

#if WITH_EDITOR
void UItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, bStackable))
	{
		if (!bStackable)
		{
			MaxStackSize = 1;
		}
	}
}
#endif

UItem::UItem()
{
	bStackable = true;
	AllowMultipleStack = true;
	ItemName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("UseActionText", "Use");
	Weight = 0.f;
	MaxStackSize = 2;
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(AARTCharacterBase* Character)
{
	OnUse(Character);
}

FString UItem::GetIdentifierString() const
{
	return GetPrimaryAssetId().ToString();
}

FPrimaryAssetId UItem::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(ItemType, GetFName());
}

#undef LOCTEXT_NAMESPACE
