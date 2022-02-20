// Copyright Epic Games, Inc. All Rights Reserved.

#include "ART.h"

#if WITH_EDITOR
DEFINE_LOG_CATEGORY(LogOrder)
DEFINE_LOG_CATEGORY(LogInventory);
#endif

UE_DEFINE_GAMEPLAY_TAG(InvStorageSlotTag, TEXT("Inventory.Slot.Storage"));
UE_DEFINE_GAMEPLAY_TAG(InvEquipSlotTag, TEXT("Inventory.Slot.Equipped"));
UE_DEFINE_GAMEPLAY_TAG(InvActiveSlotTag, TEXT("Inventory.Slot.Active"));

UE_DEFINE_GAMEPLAY_TAG(InvDropItemAbilityTag, TEXT("Ability.Inventory.DropItem"));
UE_DEFINE_GAMEPLAY_TAG(InvSwapItemSlotsAbilityTag, TEXT("Ability.Inventory.SwapItems"));
UE_DEFINE_GAMEPLAY_TAG(InvSwapPendingAbilityTag, TEXT("Ability.Inventory.SwapPending"));

#define LOCTEXT_NAMESPACE "ARTModule"

void FARTModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FARTModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FARTModule, ART)