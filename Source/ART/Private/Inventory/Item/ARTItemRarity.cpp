#include "Inventory/Item/ARTItemRarity.h"




UARTItemRarity::UARTItemRarity(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RarityName = NSLOCTEXT("RarityColor", "ErrorColor", "Error Color");
	RarityColor = FLinearColor(1, 0, 1);
}