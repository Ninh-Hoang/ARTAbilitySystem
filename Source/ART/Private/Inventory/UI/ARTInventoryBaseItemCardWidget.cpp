// Copyright 2017-2020 Puny Human, All Rights Reserved.


#include "Inventory/UI/ARTInventoryBaseItemCardWidget.h"

UARTInventoryBaseItemCardWidget::UARTInventoryBaseItemCardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UARTItemStack* UARTInventoryBaseItemCardWidget::GetItemStack() const
{
	return ItemStack;
}

void UARTInventoryBaseItemCardWidget::SetItemStack(UARTItemStack* InItemStack)
{
	ItemStack = InItemStack;
	BP_OnItemStackSet(ItemStack);
}
