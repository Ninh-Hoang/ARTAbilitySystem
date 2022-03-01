// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Generator/ARTItemGenerator_SimpleRandom.h"

#include "Inventory/ARTItemBPFunctionLibrary.h"
#include "Inventory/Item/ARTItemDefinition.h"
#include "Inventory/Mod/ARTItemStack_Mod.h"
#include "ART/ART.h"

template <typename WeightedStructType>
void SelectWeightedItems(const TArray<WeightedStructType>& WeightedRarities, int32 NumberToSelect, bool bSelectUnique, TArray< WeightedStructType>& OutSelections);

template <typename WeightedStructType>
WeightedStructType SelectSingleWeightedItem(const TArray<WeightedStructType>& WeightedRarities);


UARTItemGenerator_SimpleRandom::UARTItemGenerator_SimpleRandom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRequireUniqueModifiers = true;
	bRequireUniqueMods = true;	
}

UARTItemStack* UARTItemGenerator_SimpleRandom::GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context)
{
	if (ItemTagQuery.IsEmpty())
	{
		return nullptr;
	}

	if (RarityTable.Num() == 0)
	{
		return nullptr;
	}

	//Query some item definitions to use, we'll just select one
	
	TArray<UARTItemDefinition*> ItemDefs;
	if (!UARTItemBPFunctionLibrary::QueryMatchingItemDefinitions(ItemTagQuery, ItemDefs))
	{
		return nullptr;
	}

	//Ensure that we have item defs.  If we don't catch this (but return nullptr, because that is "safe")
	if (!ensure(ItemDefs.Num() > 0))
	{		
		return nullptr;
	}
	
	UARTItemDefinition* ItemDef = ItemDefs[FMath::RandRange(0, ItemDefs.Num() - 1)];

	if (!IsValid(ItemDef))
	{
		return nullptr;
	}

	//Select the Rarity	
	FARTItemGenerator_RaritySelector RarityEntry = SelectSingleWeightedItem(RarityTable);


	TArray<UARTItemRarity*> Rarieties;
	if (!UARTItemBPFunctionLibrary::QueryMatchingItemRarities(RarityEntry.RarityTagQuery, Rarieties))
	{
		UE_LOG(LogInventory, Warning, TEXT("UARTItemGenerator_SimpleRandom::GenerateItemStack Tried to query for item rarities but found none"));
		//return nullptr;
	}

	UARTItemRarity* Rarity = nullptr;
	if (Rarieties.Num() > 0)
	{
		Rarity = Rarieties[FMath::RandRange(0, Rarieties.Num() - 1)];
	}
		

	UARTItemStack* ItemStack = CreateNewItemStack(ItemDef, Rarity);

	//Create the Dynamic Modifier Mod, and apply dynamic modifiers to this
	if (RarityEntry.NumberOfModifiers > 0 && IsValid(DynamicModItemGenerator))
	{
		TArray< FARTItemGenerator_DynamicModifier> SelectedMods;
		SelectWeightedItems(WeightedPossibleModifiers, RarityEntry.NumberOfModifiers, bRequireUniqueModifiers, SelectedMods);

		if (UARTItemStack_Mod* Mod = Cast<UARTItemStack_Mod>(DynamicModItemGenerator->GenerateItemStack(Context)))
		{
			for (FARTItemGenerator_DynamicModifier& DynamicMod : SelectedMods)
			{
				FARTModModifierData ModifierData;
					ModifierData.Attribute = DynamicMod.Attribute;
					ModifierData.SlotApplicationRequirement = DynamicMod.SlotApplicationRequirement;
					ModifierData.ModifierOp = DynamicMod.ModifierOp;
					ModifierData.Value = FMath::RandRange(DynamicMod.MinValue, DynamicMod.MaxValue);

					Mod->Modifiers.Add(ModifierData);
			}

			//TODO: 
			//Mod->OnGenerated(Context);

			ItemStack->AddSubItemStack(Mod);
		}
	}

	if (RarityEntry.NumberOfMods > 0)
	{
		TArray< FARTItemGenerator_ModSelector> SelectedMods;
		SelectWeightedItems(WeightedPossibleMods, RarityEntry.NumberOfMods, bRequireUniqueMods, SelectedMods);
		for (FARTItemGenerator_ModSelector& SelectedMod : SelectedMods)
		{
			TArray<UARTItemGenerator*> ModGenerators;
			if (!UARTItemBPFunctionLibrary::QueryMatchingItemGenerators(SelectedMod.ModTagQuery, ModGenerators))
			{
				continue;
			}

			UARTItemGenerator* ModGenerator = ModGenerators[FMath::RandRange(0, ModGenerators.Num() - 1)];
			if (!IsValid(ModGenerator))
			{
				continue;
			}

			UARTItemStack* ModInstance = ModGenerator->GenerateItemStack(Context);
			//TODO:
			//ModInstance->OnGenerated(Context);
			ItemStack->AddSubItemStack(ModInstance);
		}
	}
				 	
	return ItemStack;
}


UARTItemGenerator_SelectGenerator::UARTItemGenerator_SelectGenerator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UARTItemStack* UARTItemGenerator_SelectGenerator::GenerateItemStack_Implementation(const FARTItemGeneratorContext& Context)
{
	if (WeightedGenerators.Num() == 0)
	{
		return nullptr;
	}

	FARTItemGenerator_GeneratorSelector Generator = SelectSingleWeightedItem(WeightedGenerators);

	if (!IsValid(Generator.ItemGen))
	{
		return nullptr;
	}
	
	return Generator.ItemGen->GenerateItemStack(Context);
}


template <typename WeightedStructType>
WeightedStructType* PickWeightedItem(const TArray<WeightedStructType>& WeightedRarities, int32 SumOfWeights);

template <typename WeightedStructType>
void SelectWeightedItems(const TArray<WeightedStructType>& WeightedRarities, int32 NumberToSelect, bool bSelectUnique, TArray< WeightedStructType>& OutSelections)
{
	//If we have no weighted rarities, then we can't really do anything here
	if (WeightedRarities.Num() == 0)
	{
		return;
	}

	//Get the sum of all weights. 
	int32 SumOfWeights = 0;
	for (const WeightedStructType& Entry : WeightedRarities)
	{
		SumOfWeights += Entry.Weight;
	}

	 //Try to select a number of items from the weighted list
	TArray<WeightedStructType*> PossibleSelection;

	for (int32 i = 0; i < NumberToSelect; i++)
	{
		int32 Tries = 0;
		WeightedStructType* GrabbedStruct = nullptr;
		do 
		{
			GrabbedStruct = PickWeightedItem(WeightedRarities, SumOfWeights);

			//If we have to pick a unique entry here, check to see if we've already grabbed it.  If we have, null out and use up a try
			if (bSelectUnique && PossibleSelection.Contains(GrabbedStruct))
			{
				GrabbedStruct = nullptr;
			}

		} while (GrabbedStruct == nullptr || ++Tries > 10); //Just give up after 10 tries.  We'll just skip adding this to the array		
		
		if (GrabbedStruct != nullptr)
		{
			PossibleSelection.Add(GrabbedStruct);
		}  		
	}

	for (WeightedStructType* Selection : PossibleSelection)
	{
		OutSelections.Add(*Selection);
	}
}



template <typename WeightedStructType>
WeightedStructType SelectSingleWeightedItem(const TArray<WeightedStructType>& WeightedRarities)
{
	if (!ensure(WeightedRarities.Num() != 0))
	{
		return WeightedStructType();
	}

	int32 SumOfWeights = 0;
	for (const WeightedStructType& Entry : WeightedRarities)
	{
		SumOfWeights += Entry.Weight;
	}

	WeightedStructType* Selected =  PickWeightedItem(WeightedRarities, SumOfWeights);

	if (Selected != nullptr)
	{
		return *Selected;
	}

	//If we don't find a rarity above, just grab the first element of the array
	return WeightedRarities[0];

}

template <typename WeightedStructType>
WeightedStructType* PickWeightedItem(const TArray<WeightedStructType>& WeightedRarities, int32 SumOfWeights)
{
	int32 RandomRoll = FMath::RandRange(0, SumOfWeights);

	for (const WeightedStructType& Struct : WeightedRarities)
	{
		if (RandomRoll < Struct.Weight)
		{
			return const_cast<WeightedStructType*>(&Struct);
		}
		RandomRoll -= Struct.Weight;
	}

	return nullptr;
}
