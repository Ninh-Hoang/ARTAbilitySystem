// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ARTItemBPFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/AssetManager.h"
#include "Inventory/ARTInventoryDeveloperSettings.h"
#include "Inventory/Component/ARTInventoryComponent_Active.h"
#include "Inventory/Generator/ARTItemGenerator.h"
#include "Inventory/Interfaces/ARTInventoryInterface.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/Item/ARTItemRarity.h"
#include "Inventory/Item/ARTItemStackWorldObject.h"
#include "Inventory/Mod/ARTItemStack_SlotContainer.h"
#include "Kismet/GameplayStatics.h"

UARTItemBPFunctionLibrary::UARTItemBPFunctionLibrary()
	: Super()
{

}


FString UARTItemBPFunctionLibrary::GetAttributeString(FGameplayAttribute Attribute)
{
	return Attribute.GetName();
}


template <typename T>
bool UARTItemBPFunctionLibrary::QueryMatchingAssets(FGameplayTagQuery Query, TArray<T*>& OutResults)
{	
	UClass* AssetClass = T::StaticClass();

	//If we don't implement the GameplayTagAssetInterface, dont bother seARThing deeper.
	if (!AssetClass->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
	{
		return false;
	}

	//Find the Primary Asset Type for T, if we have it one
	TArray<FPrimaryAssetTypeInfo> AssetTypeInfoList;
	UAssetManager::Get().GetPrimaryAssetTypeInfoList(AssetTypeInfoList);

	FPrimaryAssetType PrimaryAssetType;
	for (auto AssetTypeInfo : AssetTypeInfoList)
	{
		if (AssetClass->IsChildOf(AssetTypeInfo.AssetBaseClassLoaded))
		{
			PrimaryAssetType = FPrimaryAssetType(AssetTypeInfo.PrimaryAssetType);
			break;
		}
	}

	if (!PrimaryAssetType.IsValid())
	{
		return false;
	}

	//Get All objects of this type
	TArray<UObject*> Objects;
	bool bSuccess = UAssetManager::Get().GetPrimaryAssetObjectList(PrimaryAssetType, Objects);

	//And match them against the query
	for (int32 i  = 0; i < Objects.Num(); i++)
	{		
		UObject* Obj = Objects[i];
		if (UClass* Class = Cast<UClass>(Obj))
		{
			Obj = Class->GetDefaultObject();
		}

		if (IGameplayTagAssetInterface* InterfacePtr = Cast< IGameplayTagAssetInterface>(Obj))
		{
			FGameplayTagContainer TagContainer;
			InterfacePtr->GetOwnedGameplayTags(TagContainer);

			if (Query.Matches(TagContainer))
			{
				OutResults.Add(Cast<T>(Obj));
			}
		}		
	}

	return bSuccess;
}

template <typename T>
bool UARTItemBPFunctionLibrary::QueryMatchingAssetClasses(FGameplayTagQuery Query, TArray<T*>& OutResults)
{
	TArray<T*> DefaultObjects;
	bool bSuccess = QueryMatchingAssets<T>(Query, DefaultObjects);

	if (bSuccess)
	{
		for (T* Object : DefaultObjects)
		{
			OutResults.Add(Object);
		}
	}

	return bSuccess;
}



bool UARTItemBPFunctionLibrary::QueryMatchingItemDefinitions(FGameplayTagQuery Query, TArray<UARTItemDefinition*>& OutItemDefinitions)
{
	return QueryMatchingAssetClasses<UARTItemDefinition>(Query, OutItemDefinitions);
}

bool UARTItemBPFunctionLibrary::QueryMatchingItemRarities(FGameplayTagQuery Query, TArray<UARTItemRarity*>& OutItemRarities)
{	
	return QueryMatchingAssets<UARTItemRarity>(Query, OutItemRarities);
}

bool UARTItemBPFunctionLibrary::QueryMatchingItemGenerators(FGameplayTagQuery Query, TArray<UARTItemGenerator*>& OutItemGenerators)
{
	return QueryMatchingAssets<UARTItemGenerator>(Query, OutItemGenerators);
}

UARTInventoryComponent* UARTItemBPFunctionLibrary::GetInventoryComponent(AActor* Actor, bool bSearchComponents /*= false*/)
{
	if (IARTInventoryInterface* InventoryInterface = Cast<IARTInventoryInterface>(Actor))
	{
		return InventoryInterface->GetInventoryComponent();
	}

	if (bSearchComponents)
	{
		if (UARTInventoryComponent* IC = Cast<UARTInventoryComponent>(Actor->GetComponentByClass(UARTInventoryComponent::StaticClass())))
		{
			return IC;
		}
	}
		
	return nullptr;
}

UARTInventoryComponent_Storage* UARTItemBPFunctionLibrary::GetStorageInventoryComponent(AActor* Actor, bool bSearchComponents)
{
	return Cast<UARTInventoryComponent_Storage>(GetInventoryComponent(Actor, bSearchComponents));
}

UARTInventoryComponent_Equippable* UARTItemBPFunctionLibrary::GetEquippableInventoryComponent(AActor* Actor)
{
	return Cast<UARTInventoryComponent_Equippable>(GetInventoryComponent(Actor));
}

UARTInventoryComponent_Active* UARTItemBPFunctionLibrary::GetActiveInventoryComponent(AActor* Actor, bool bSearchComponents)
{
	return Cast<UARTInventoryComponent_Active>(GetInventoryComponent(Actor, bSearchComponents));
}

bool UARTItemBPFunctionLibrary::TwoItemCanStack(UARTItemStack* TargetStack, UARTItemStack* SourceStack)
{
	if(!TargetStack || !SourceStack) return false;
	if(TargetStack->GetItemDefinition() != SourceStack->GetItemDefinition()) return false;
	if(TargetStack->GetStackSize() >= TargetStack->GetItemDefinition()->MaxStackSize) return false;
	return true;
}

bool UARTItemBPFunctionLibrary::RemoveItemInSlot(const FARTItemSlotRef& ItemSlot)
{
	if(!IsValid(ItemSlot)) return false;

	if(UARTItemStack_SlotContainer* Container = ItemSlot.ParentStack.Get())
	{
		return Container->RemoveItemFromContainer(ItemSlot);
	}
	if(UARTInventoryComponent* Inventory = ItemSlot.ParentInventory.Get())
	{
		return Inventory->RemoveItemFromInventory(ItemSlot);
	}
	//we should never touch here
	check(0);
	return false;
}

bool UARTItemBPFunctionLibrary::CanAcceptSlotItem_AssumeEmptySlot(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot)
{
	if(!IsValid(ToSlot)) return false;
	if(UARTItemStack_SlotContainer* Container = ToSlot.ParentStack.Get())
	{
		return Container->AcceptsItem_AssumeEmptySlot(GetItemFromSlot(FromSlot), ToSlot);
	}
	if(UARTInventoryComponent* Inventory = ToSlot.ParentInventory.Get())
	{
		return Inventory->AcceptsItem_AssumeEmptySlot(GetItemFromSlot(FromSlot), ToSlot);
	}
	//we should never touch here
	check(0);
	return false;
}

bool UARTItemBPFunctionLibrary::CanAcceptItem_AssumeEmptySlot(UARTItemStack* Item, const FARTItemSlotRef& ToSlot)
{
	if(!IsValid(ToSlot)) return false;
	if(UARTItemStack_SlotContainer* Container = ToSlot.ParentStack.Get())
	{
		return Container->AcceptsItem_AssumeEmptySlot(Item, ToSlot);
	}
	if(UARTInventoryComponent* Inventory = ToSlot.ParentInventory.Get())
	{
		return Inventory->AcceptsItem_AssumeEmptySlot(Item, ToSlot);
	}
	//we should never touch here
	check(0);
	return false;
}

bool UARTItemBPFunctionLibrary::DoesItemContainSlot(UARTItemStack* Item, const FARTItemSlotRef& Slot)
{
	if(!IsValid(Slot) || !Item) return false;
	UARTItemStack* ParentStack = Slot.ParentStack.Get();
	while(ParentStack)
	{
		if(ParentStack == Item) return true;
		ParentStack = ParentStack->GetParentStack();
	}
	return false;
}

bool UARTItemBPFunctionLibrary::IsValidItemSlot(const FARTItemSlotRef& ItemSlotRef)
{
	return IsValid(ItemSlotRef);
}

bool UARTItemBPFunctionLibrary::SwapItemSlot(const FARTItemSlotRef& FromSlot, const FARTItemSlotRef& ToSlot)
{
	if(!IsValid(FromSlot) || !IsValid(ToSlot)) return false;

	/*if(FromSlot.ParentInventory.IsValid() && FromSlot.ParentInventory.Get() == ToSlot.ParentInventory.Get())
	{
		return FromSlot.ParentInventory.Get()->SwapItemSlots(FromSlot, ToSlot);
	}
	if(FromSlot.ParentStack.IsValid() && FromSlot.ParentStack.Get() == ToSlot.ParentStack.Get())
	{
		return FromSlot.ParentStack.Get()->SwapItemSlots(FromSlot, ToSlot);
	}*/

	if(FromSlot.ParentInventory.IsValid()) return FromSlot.ParentInventory.Get()->SwapItemSlots(FromSlot, ToSlot);
	if(ToSlot.ParentInventory.IsValid()) return ToSlot.ParentInventory.Get()->SwapItemSlots(FromSlot, ToSlot);
	if(FromSlot.ParentStack.IsValid()) return FromSlot.ParentStack.Get()->SwapItemSlots(FromSlot, ToSlot);
	if(ToSlot.ParentStack.IsValid()) return ToSlot.ParentStack.Get()->SwapItemSlots(FromSlot, ToSlot);
	check(0);
	return false;
}

UARTItemStack* UARTItemBPFunctionLibrary::GetItemFromSlot(const FARTItemSlotRef& ItemSlotRef)
{
	if (!IsValid(ItemSlotRef))
	{
		return nullptr;
	}

	//use slot container instead if parent stack exist
	if(ItemSlotRef.ParentStack.IsValid())
	{
		if(UARTItemStack_SlotContainer* Container = Cast<UARTItemStack_SlotContainer>(ItemSlotRef.ParentStack.Get()))
		{
			return Container->GetItemSlot(ItemSlotRef).ItemStack;
		}
	}

	return ItemSlotRef.ParentInventory->GetItemSlot(ItemSlotRef).ItemStack;
}

bool UARTItemBPFunctionLibrary::EqualEqual_FARTItemSlotRef(const FARTItemSlotRef& ItemSlotRef, const FARTItemSlotRef& OtherItemSlotRef)
{
	return ItemSlotRef == OtherItemSlotRef;
}

bool UARTItemBPFunctionLibrary::NotEqual_FARTItemSlotRef(const FARTItemSlotRef& ItemSlotRef,
	const FARTItemSlotRef& OtherItemSlotRef)
{
	return ItemSlotRef != OtherItemSlotRef;
}

UARTInventoryComponent* UARTItemBPFunctionLibrary::GetInventoryFromSlot(const FARTItemSlotRef& ItemSlotRef)
{
	return ItemSlotRef.ParentInventory.Get();
}

bool UARTItemBPFunctionLibrary::IsValidInventoryQuery(const FARTSlotQueryHandle& Query)
{
	return Query.Query.IsValid();
}

FARTSlotQueryHandle UARTItemBPFunctionLibrary::MakeSlotQueryHandle_GameplayTagQuery(const FGameplayTagQuery& SlotQuery,
	const FGameplayTagQuery& ItemQuery)
{
	FARTSlotQuery Query;
	Query.SlotTypeQuery = SlotQuery;
	Query.ItemTypeQuery = ItemQuery;

	FARTSlotQuery* NewQuery = new FARTSlotQuery(Query);

	FARTSlotQueryHandle QueryHandle;
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(NewQuery);
	return QueryHandle;
}

FARTSlotQueryHandle UARTItemBPFunctionLibrary::MakeSlotQueryHandle_SlotWithItem(
	const TSubclassOf<UARTItemDefinition> ItemDefinition, const TEnumAsByte<EItemStackCount::Type> StackCount,
	const FGameplayTagContainer& ItemRequiredTags, const FGameplayTagContainer& ItemBlockedTags)
{
	FARTSlotQuery_SlotWithItem Query;
	Query.ItemDefinition = ItemDefinition;
	Query.StackCount = StackCount;
	Query.ItemRequiredTags = ItemRequiredTags;
	Query.ItemBlockedTags = ItemBlockedTags;

	FARTSlotQuery* NewQuery = new FARTSlotQuery_SlotWithItem(Query);
	FARTSlotQueryHandle QueryHandle;
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(NewQuery);
	return QueryHandle;
}

FARTSlotQueryHandle UARTItemBPFunctionLibrary::MakeSlotQueryHandle_SlotCanAcceptItem(
	const TEnumAsByte< EItemExistence::Type> ItemExist,
	UARTItemStack* ContextItemStack)
{
	FARTSlotQuery_SlotCanAcceptItem Query;
	Query.ItemExist = ItemExist;
	Query.ContextItemStack = ContextItemStack;

	FARTSlotQuery* NewQuery = new FARTSlotQuery_SlotCanAcceptItem(Query);
	FARTSlotQueryHandle QueryHandle;
	QueryHandle.Query = TSharedPtr<FARTSlotQuery>(NewQuery);
	return QueryHandle;
}

void UARTItemBPFunctionLibrary::CopyAttributeSet(UAttributeSet* Src, UAttributeSet* Destination)
{
	//Have to be the same class to copy
	if (Src->GetClass() != Destination->GetClass())
	{
		return;
	}

	for (TFieldIterator<FProperty> PropertyIt(Src->GetClass(), EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		FGameplayAttribute Attribute(Property);

		float AttributeValue = Attribute.GetNumericValue(Src);
		Attribute.SetNumericValueChecked(AttributeValue, Destination);
	}
}

bool UARTItemBPFunctionLibrary::ASCHasAttributeSet(UAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass)
{
	for (UAttributeSet* AttributeSet : ASC->GetSpawnedAttributes_Mutable())
	{
		if (AttributeSet->GetClass() == AttributeSetClass)
		{
			return true;
		}
	}

	return false;
}

bool UARTItemBPFunctionLibrary::ASCAddInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet)
{
	if (ASC->GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}
	if (!IsValid(AttributeSet))
	{
		return false;
	}
	//We don't want to double up attributes.  If we already have this attribute set added we need to fix that
	ensure(!ASC->GetSpawnedAttributes_Mutable().Contains(AttributeSet));

	ASC->GetSpawnedAttributes_Mutable().AddUnique(AttributeSet);
	ASC->bIsNetDirty = true;

	return false;
}

bool UARTItemBPFunctionLibrary::ASCRemoveInstancedAttributeSet(UAbilitySystemComponent* ASC, UAttributeSet* AttributeSet)
{
	if (!IsValid(AttributeSet))
	{
		return false;
	}
	//TODO: Reset the Attribute Set

	ASC->GetSpawnedAttributes_Mutable().Remove(AttributeSet);
	ASC->bIsNetDirty = true;

	return true;
}

float UARTItemBPFunctionLibrary::GetActiveAttributeFromItemSlot(const FARTItemSlotRef& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute)
{
	bSuccessfullyFoundAttribute = false;
	UARTInventoryComponent_Active* Inventory = Cast<UARTInventoryComponent_Active>(ItemSlotRef.ParentInventory);

	if (!IsValid(Inventory) || !Inventory->IsValidItemSlot(ItemSlotRef))
	{
		return 0.0f;
	}	
	
	if(!Attribute.IsValid())
	{
		return 0.0f;
	}
	
	FARTInventoryItemInfoEntry* Entry = Inventory->ActiveItemAbilityInfos.FindByPredicate([ItemSlotRef](FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef == ItemSlotRef;
	});

	if (Entry == nullptr)
	{
		return 0.0f;
	}

	for (UAttributeSet* AttributeSet : (*Entry).EquippedItemInfo.InstancedAttributeSets)
	{
		if (!IsValid(AttributeSet))
		{
			continue;
		}

		if (AttributeSet->GetClass() == Attribute.GetAttributeSetClass())
		{
			bSuccessfullyFoundAttribute = true;
			return Attribute.GetNumericValue(AttributeSet);
		}
	}  	
	
	return 0.0f;
}

float UARTItemBPFunctionLibrary::GetEquippedAttributeFromItemSlot(const FARTItemSlotRef& ItemSlotRef, FGameplayAttribute Attribute, bool& bSuccessfullyFoundAttribute)
{
	bSuccessfullyFoundAttribute = false;
	UARTInventoryComponent_Equippable* Inventory = Cast<UARTInventoryComponent_Equippable>(ItemSlotRef.ParentInventory);

	if (!IsValid(Inventory) || !Inventory->IsValidItemSlot(ItemSlotRef))
	{
		return 0.0f;
	}
	
	FARTInventoryItemInfoEntry* Entry = Inventory->EquippedItemAbilityInfos.FindByPredicate([ItemSlotRef](FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef == ItemSlotRef;
	});

	if (Entry == nullptr)
	{
		return 0.0f;
	}

	for (UAttributeSet* AttributeSet : (*Entry).EquippedItemInfo.InstancedAttributeSets)
	{
		if (AttributeSet->GetClass() == Attribute.GetAttributeSetClass())
		{
			bSuccessfullyFoundAttribute = true;
			return Attribute.GetNumericValue(AttributeSet);
		}
	}  

	return 0.0f;
}

class AARTItemStackWorldObject* UARTItemBPFunctionLibrary::SpawnWorldItem(UObject* WorldContextObject, UARTItemStack* ItemStack, const FTransform& Transform)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (IsValid(World))
	{
		TSubclassOf<AARTItemStackWorldObject> WorldStackClass = AARTItemStackWorldObject::StaticClass();
		if (IsValid(GetDefault<UARTInventoryDeveloperSettings>()->ItemStackWorldObjectClass))
		{
			WorldStackClass = GetDefault<UARTInventoryDeveloperSettings>()->ItemStackWorldObjectClass;
		}

		AARTItemStackWorldObject* StackObject = World->SpawnActorDeferred<AARTItemStackWorldObject>(WorldStackClass, Transform);
		if (IsValid(StackObject))
		{
			StackObject->SetInventoryStack(ItemStack);
			UGameplayStatics::FinishSpawningActor(StackObject, Transform);
			return StackObject;
		}
	}
	return nullptr;
}

UARTItemUIData_ItemDefinition* UARTItemBPFunctionLibrary::GetUIDataFromItemDefinition(UARTItemDefinition* ItemDefinition)
{
	if (IsValid(ItemDefinition))
	{
		return ItemDefinition->UIData;
	}
	return nullptr;	
}