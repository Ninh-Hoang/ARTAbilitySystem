// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Component/ARTInventoryComponent_Equippable.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ART/ART.h"
#include "Inventory/ARTItemBPFunctionLibrary.h"
#include "Inventory/ARTItemStack.h"
#include "Inventory/Input/ARTItemInputBinder.h"
#include "Inventory/Item/Definition/ARTItemDefinition_Equipment.h"
#include "Inventory/Mod/ARTItemStack_Mod.h"
#include "Net/UnrealNetwork.h"

UARTInventoryComponent_Equippable::UARTInventoryComponent_Equippable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UARTInventoryComponent_Equippable::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		OnItemSlotChange.AddDynamic(this, &UARTInventoryComponent_Equippable::OnItemEquipped);
	}
}

void UARTInventoryComponent_Equippable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UARTInventoryComponent_Equippable, EquippedItemAbilityInfos, COND_OwnerOnly);
}

void UARTInventoryComponent_Equippable::OnItemEquipped(class UARTInventoryComponent* Inventory, const FARTInventoryItemSlotReference& ItemSlotRef, UARTItemStack* ItemStack, UARTItemStack* PreviousItemStack)
{
	if (IsValid(PreviousItemStack) && IsEquippedItemSlot(ItemSlotRef))
	{
		MakeItemUnequipped_Internal(ItemSlotRef, PreviousItemStack);
	}
	if (IsValid(ItemStack) && IsEquippedItemSlot(ItemSlotRef))
	{
		MakeItemEquipped_Internal(ItemSlotRef, ItemStack);	
	}
}

bool UARTInventoryComponent_Equippable::IsEquippedItemSlot(const FARTInventoryItemSlotReference& ItemSlotRef)
{
	if (!ItemSlotRef.SlotTags.HasTagExact(InvEquipSlotTag))
	{
		return false;
	}
	return true;
}

bool UARTInventoryComponent_Equippable::MakeItemEquipped_Internal(const FARTInventoryItemSlotReference& ItemSlot)
{
	if (!IsValidItemSlot(ItemSlot))
	{
		return false;
	}
	UARTItemStack* ItemStack = GetItemInSlot(ItemSlot);

	return MakeItemEquipped_Internal(ItemSlot, ItemStack);
}

bool UARTInventoryComponent_Equippable::MakeItemEquipped_Internal(const FARTInventoryItemSlotReference& ItemSlot, UARTItemStack* ItemStack)
{
	if (!IsValid(ItemStack))
	{
		return false;
	}
	
	TSubclassOf<UARTItemDefinition_Equipment> ItemDefinition(ItemStack->GetItemDefinition());
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	FARTInventoryItemInfoEntry* Entry = EquippedItemAbilityInfos.FindByPredicate([ItemSlot](FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef.SlotId == ItemSlot.SlotId;
	});
	if (Entry == nullptr)
	{
		Entry = &EquippedItemAbilityInfos.Add_GetRef(FARTInventoryItemInfoEntry(ItemSlot));
	}

	bool bSuccess = ApplyAbilityInfo_Internal(ItemDefinition.GetDefaultObject()->EquippedItemAbilityInfo, (*Entry).EquippedItemInfo, ItemStack);

	if (bSuccess)
	{
		//Apply this item's modifiers
		ApplyMods(ItemStack, ItemSlot);

		OnEquippedItem.Broadcast(this, ItemSlot, ItemStack);
	}

	//if there are any post-equip things to do aside from apply abilities, do them here.
	return 	bSuccess;
}

bool UARTInventoryComponent_Equippable::MakeItemUnequipped_Internal(const FARTInventoryItemSlotReference& ItemSlot)
{
	if (!IsValidItemSlot(ItemSlot))
	{
		return false;
	}
	UARTItemStack* ItemStack = GetItemInSlot(ItemSlot);

	return MakeItemUnequipped_Internal(ItemSlot, ItemStack);
}

bool UARTInventoryComponent_Equippable::MakeItemUnequipped_Internal(const FARTInventoryItemSlotReference& ItemSlot, UARTItemStack* ItemStack)
{
	//Ensure we have valid data for this item
	if (!IsValid(ItemStack))
	{
		return false;
	}

	TSubclassOf<UARTItemDefinition_Equipment> ItemDefinition(ItemStack->GetItemDefinition());
	if (!IsValid(ItemDefinition))
	{
		return false;
	}

	FARTInventoryItemInfoEntry* Entry = EquippedItemAbilityInfos.FindByPredicate([ItemSlot](FARTInventoryItemInfoEntry& x) {
		return x.ItemSlotRef.SlotId == ItemSlot.SlotId;
	});
	if (Entry == nullptr)
	{
		return false;
	}


	RemoveMods(ItemStack, ItemSlot);
	//Clear the active ability info
	bool bSuccess = ClearAbilityInfo_Internal(ItemDefinition.GetDefaultObject()->EquippedItemAbilityInfo, (*Entry).EquippedItemInfo);

	//IF there is more that needs to be done aside from ability info, put it here
	if (bSuccess)
	{
		OnUnEquippedItem.Broadcast(this, ItemSlot, ItemStack);
	}

	return 	bSuccess;
}

bool UARTInventoryComponent_Equippable::ApplyAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto, UARTItemStack* AbilitySource)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		//Setup the Item Info, duplicating attribute sets if we have to
		for (auto AttributeSetClass : AbilityInfo.AttributeSetsToAdd)
		{
			//Find an attribute set with the same key
			UAttributeSet** ContainedAttributeSet = StoreInto.InstancedAttributeSets.FindByPredicate([=](UAttributeSet* Key) {
				return Key->GetClass() == AttributeSetClass.Get();
			});
			if (ContainedAttributeSet != nullptr)	 //If it exists, we've got it!
			{
				continue;
			}



			//Otherwise, create a new one
			UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);

			//and init the attributes
			for (auto KV : AbilityInfo.AttributeInitalizers)
			{
				FGameplayAttribute Attribute = KV.Key;
				float val = KV.Value;

				if (Attribute.GetAttributeSetClass() == NewAttributeSet->GetClass())
				{					
					if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Attribute.GetUProperty()))
					{
						void* ValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(NewAttributeSet);
						NumericProperty->SetFloatingPointPropertyValue(ValuePtr, val);
					}
					else if (FStructProperty* StructProperty = CastField<FStructProperty>(Attribute.GetUProperty()))
					{
						FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(NewAttributeSet);
						if (DataPtr)
						{
							DataPtr->SetBaseValue(val);
							DataPtr->SetCurrentValue(val);
						}
					}
				}
			}

			//and then tell watchers that a new attribute set has been created
			OnAttributeSetCreated.Broadcast(this, NewAttributeSet, AbilitySource);

			StoreInto.InstancedAttributeSets.Add(NewAttributeSet);
		}
	}

	
	if (UAbilitySystemComponent* ASC = GetOwnerAbilitySystem())
	{
		//Add any loose tags first, that way any abilities or effects we add later behave properly with the tags  
		ASC->AddLooseGameplayTags(AbilityInfo.GrantedTags, 1);

		if (GetOwnerRole() == ROLE_Authority)
		{
			//Add any attribute sets we have
			for (UAttributeSet* AttributeSet : StoreInto.InstancedAttributeSets)
			{				
				if (!UARTItemBPFunctionLibrary::ASCHasAttributeSet(ASC, AttributeSet->GetClass()))
				{
					AttributeSet->Rename(nullptr, GetOwner());
					UARTItemBPFunctionLibrary::ASCAddInstancedAttributeSet(ASC, AttributeSet);
					
				}
			}
					

			//Add all the active abilities for the ability slots we have
			for (auto AbilityInfoStruct : AbilityInfo.ActiveAbilityEntries)
			{
				TSubclassOf<UGameplayAbility> AbilityClass = AbilityInfoStruct.ActiveAbilityClass;
				UARTItemInputBinder* InputBinder = AbilityInfoStruct.InputBinder;
				if (!IsValid(AbilityClass) || !IsValid(InputBinder))
				{
					continue;
				}				
				
				int32 InputIndex = InputBinder->GetInputBinding(ASC, AbilityClass);
				FGameplayAbilitySpec Spec(AbilityClass.GetDefaultObject(), 1, InputIndex, AbilitySource);
							

				FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				StoreInto.AddedAbilities.Add(Handle);
			}
			//and add any extras we have
			for (auto ExtraAbility : AbilityInfo.ExtraAbilities)
			{
				//If an ability exists already, then don't bother adding it
				FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(ExtraAbility);
				if (AbilitySpec != nullptr && !(!!AbilitySpec->PendingRemove))
				{
					continue;
				}

				FGameplayAbilitySpec Spec(ExtraAbility.GetDefaultObject(), 1, INDEX_NONE, AbilitySource);

				FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				StoreInto.AddedAbilities.Add(Handle);
			}

			

			//Add any GameplayEffects we have
			for (auto EffectClass : AbilityInfo.AddedGameplayEffects)
			{
				for (auto& Handle : StoreInto.AddedGameplayEffects)
				{
					if (ASC->GetGameplayEffectDefForHandle(Handle) == EffectClass->GetDefaultObject<UGameplayEffect>())
					{
						continue;
					}
				}

				FGameplayEffectContextHandle ECH(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
				ECH.AddInstigator(GetOwner(), GetOwner());
				FGameplayEffectSpec Spec(EffectClass->GetDefaultObject<UGameplayEffect>(), ECH, 1);

				
				FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(Spec);
				//Store any non-instant handles we get.  Instant Gameplay Effects will fall off right away so we don't need to remove them later
				//Really, items shouldn't use instant GEs.
				if (Spec.Def->DurationPolicy != EGameplayEffectDurationType::Instant)
				{
					StoreInto.AddedGameplayEffects.Add(Handle);
				}
			}

			ASC->bIsNetDirty = true;
		}

		
	}

	return true;
}

bool UARTInventoryComponent_Equippable::ClearAbilityInfo_Internal(const FARTItemDefinition_AbilityInfo& AbilityInfo, FARTEquippedItemInfo& StoreInto)
{
	//Remove any abilities this item adds
	
	if (UAbilitySystemComponent* ASC = GetOwnerAbilitySystem())
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			//Remove all of the gameplay effects
			{
				for (auto Handle : StoreInto.AddedGameplayEffects)
				{
					ASC->RemoveActiveGameplayEffect(Handle);
				}
			}

			//Remove all the abilities we added
			{
				for (auto Handle : StoreInto.AddedAbilities)
				{
					ASC->CancelAbilityHandle(Handle);
					ASC->ClearAbility(Handle);
				}
				StoreInto.AddedAbilities.Empty();
			}

			//Remove Attribute Sets			
			for (UAttributeSet* AttributeSet : StoreInto.InstancedAttributeSets)
			{
				UARTItemBPFunctionLibrary::ASCRemoveInstancedAttributeSet(ASC, AttributeSet);				
			}
			
		}

		//Remove any extra granted tags from the player				
		ASC->RemoveLooseGameplayTags(AbilityInfo.GrantedTags);
	}
	

	return true;
}

void UARTInventoryComponent_Equippable::ApplyMods(UARTItemStack* ItemStack, const FARTInventoryItemSlotReference& ItemSlot)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	//Get our Ability System
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystem();
	

	//If the Ability System doesn't exist we can't really do much here
	if (!IsValid(ASC))
	{
		return;
	}
	TArray<UARTItemStack*> SubItemStacks;
	ItemStack->GetSubItems(SubItemStacks);
	for (UARTItemStack* SubStack : SubItemStacks)
	{
		if (UARTItemStack_Mod* Mod = Cast<UARTItemStack_Mod>(SubStack))
		{
			TArray<FARTModModifierData> ModifierData;
			
			for (auto Modifier : Mod->Modifiers)
			{
				ModifierData.Add(Modifier);
			}
			

			for (FARTModModifierData& Modifier : ModifierData)
			{
				if (!Modifier.SlotApplicationRequirement.Matches(ItemSlot.SlotTags))
				{
					continue;
				}

				//Apply the Modifier to the ASC
				ASC->ApplyModToAttribute(Modifier.Attribute, Modifier.ModifierOp, Modifier.Value);
			}
		}
	}
	
}

void UARTInventoryComponent_Equippable::RemoveMods(UARTItemStack* ItemStack, const FARTInventoryItemSlotReference& ItemSlot)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (!IsValid(ItemStack))
	{
		return;
	}

	//Get our Ability System
	UAbilitySystemComponent* ASC = GetOwnerAbilitySystem();
	//If the Ability System doesn't exist we can't really do much here
	if (!IsValid(ASC))
	{
		return;
	}
	TArray<UARTItemStack*> SubItemStacks;
	ItemStack->GetSubItems(SubItemStacks);
	for (UARTItemStack* SubStack : SubItemStacks)
	{
		if (UARTItemStack_Mod* Mod = Cast<UARTItemStack_Mod>(SubStack))
		{


			for (FARTModModifierData& Modifier : Mod->Modifiers)
			{
				if (!ASC->HasAttributeSetForAttribute(Modifier.Attribute))
				{
					continue;
				}

				if (!Modifier.SlotApplicationRequirement.Matches(ItemSlot.SlotTags))
				{
					continue;
				}

				//Figure out the opposite of what we applied
				float Value = Modifier.Value;
				TEnumAsByte<EGameplayModOp::Type> ModOp = Modifier.ModifierOp;

				//Invert Additive 
				if (Modifier.ModifierOp == EGameplayModOp::Additive)
				{
					Value = Value * -1;
				}
				//Invert Mult and Div
				if (ModOp == EGameplayModOp::Multiplicitive)
				{
					ModOp = EGameplayModOp::Division;
				}
				else if (ModOp == EGameplayModOp::Division)
				{
					ModOp = EGameplayModOp::Multiplicitive;
				}

				//Apply the Modifier to the ASC
				ASC->ApplyModToAttribute(Modifier.Attribute, ModOp, Value);
			}
		}
	}
}

void UARTInventoryComponent_Equippable::Debug_Internal(struct FInventoryComponentDebugInfo& Info)
{
	Super::Debug_Internal(Info);
	/*
	//Draw the equipment
	{
		FGameplayTag EquipSlotTag = GetDefault<UARTInventoryDeveloperSettings>()->EquippedSlotTag;

		DebugLine(Info, FString::Printf(TEXT("Equipment Slots (Slots: %d)"), GetInventorySize()), 0.0f, 0.0f);
		TArray< FARTInventoryItemSlotReference> QueriedEquipmentSlots;

		Query_GetAllSlots(FARTItemQuery::QuerySlotMatchingTag(EquipSlotTag), QueriedEquipmentSlots);

		for (auto InventorySlot : QueriedEquipmentSlots)
		{
			FARTInventoryItemSlot Slot = this->GetItemSlot(InventorySlot);
			TArray<FString> DebugStrings;
			Slot.ToDebugStrings(DebugStrings, false);
			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Yellow);
			}
			DebugLine(Info, DebugStrings[0], 4.0f, 0.0f);

			if (Info.Canvas)
			{
				Info.Canvas->SetDrawColor(FColor::Cyan);
			}
			for (int i = 1; i < DebugStrings.Num(); i++)
			{
				DebugLine(Info, DebugStrings[i], 12.0f, 0.0f);
			}
		}
	}
	*/
}

bool UARTInventoryComponent_Equippable::Query_GetAllEquippableSlots(TArray<FARTInventoryItemSlotReference>& OutSlotRefs)
{
	return Query_GetAllSlots(FARTItemQuery::QuerySlotMatchingTag(InvEquipSlotTag), OutSlotRefs);
}