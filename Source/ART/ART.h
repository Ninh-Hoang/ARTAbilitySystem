// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "NativeGameplayTags.h"
#include "Networking.h"
#include "UObject/ObjectMacros.h"

#if WITH_EDITOR
DECLARE_LOG_CATEGORY_EXTERN(LogOrder, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Display, All);
#endif

DECLARE_STATS_GROUP(TEXT("Order"), STATGROUP_Order, STATCAT_Advanced);

#define ACTOR_ROLE_STRING *(FindObject<UEnum>(ANY_PACKAGE, TEXT("ENetRole"), true)->GetNameStringByValue(GetLocalRole()))
#define GET_ACTOR_ROLE_STRING(Actor) *(FindObject<UEnum>(ANY_PACKAGE, TEXT("ENetRole"), true)->GetNameStringByValue(Actor->GetLocalRole()))

#define COLLISION_ABILITY						ECollisionChannel::ECC_GameTraceChannel1
#define COLLISION_PROJECTILE					ECollisionChannel::ECC_GameTraceChannel2
#define COLLISION_ABILITY_OVERLAP_PROJECTILE		ECollisionChannel::ECC_GameTraceChannel3
#define COLLISION_PICKUP						ECollisionChannel::ECC_GameTraceChannel4
#define COLLISION_INTERACTABLE					ECollisionChannel::ECC_GameTraceChannel5

UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvStorageSlotTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvEquipSlotTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvActiveSlotTag);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvDropItemAbilityTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvSwapItemSlotsAbilityTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InvSwapPendingAbilityTag);

UENUM(BlueprintType)
enum class EARTHitReactDirection : uint8
{
	// 0
	None UMETA(DisplayName = "None"),
	// 1
	Left UMETA(DisplayName = "Left"),
	// 2
	Front UMETA(DisplayName = "Front"),
	// 3
	Right UMETA(DisplayName = "Right"),
	// 4
	Back UMETA(DisplayName = "Back")
};

UENUM(BlueprintType)
enum class EARTAbilityInputID : uint8
{
	// 0 None
	None UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel UMETA(DisplayName = "Cancel"),
	// 3 Q
	Q UMETA(DisplayName = "Q"),
	// 4 E
	E UMETA(DisplayName = "E"),
	// 5 R
	R UMETA(DisplayName = "R"),
	// 6 F
	F UMETA(DisplayName = "F"),
	// 7 G
	G UMETA(DisplayName = "G"),
	// 9 LMB	
	LMB UMETA(DisplayName = "LMB"),
	// 10 RMB	
	RMB UMETA(DisplayName = "RMB	"),
	// 11 Sprint
	Shift UMETA(DisplayName = "Shift"),
	// 12 Jump
	Space UMETA(DisplayName = "Space")
};

class FARTModule : public IModuleInterface
{
	public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
