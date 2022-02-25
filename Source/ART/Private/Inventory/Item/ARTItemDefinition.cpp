
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/ARTItemStack.h"

const FName ItemDefinitionName = TEXT("ARTItemDefinition");

UARTItemDefinition::UARTItemDefinition(): Super()
{
	MaxStackSize = 1;
	DefaultItemStackClass = UARTItemStack::StaticClass();
}

FPrimaryAssetId UARTItemDefinition::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UClass* SearchNativeClass = GetClass();

		while (SearchNativeClass && !SearchNativeClass->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic))
		{
			SearchNativeClass = SearchNativeClass->GetSuperClass();
		}

		if (SearchNativeClass && SearchNativeClass != GetClass())
		{
			// If blueprint, return native class and asset name
			return FPrimaryAssetId(ItemDefinitionName, FPackageName::GetShortFName(GetOutermost()->GetFName()));
		}

		// Native CDO, return nothing
		return FPrimaryAssetId();
	}

	return  GetClass()->GetDefaultObject()->GetPrimaryAssetId();
}

