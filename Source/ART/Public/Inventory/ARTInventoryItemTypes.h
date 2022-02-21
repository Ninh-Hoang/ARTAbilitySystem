#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "ARTInventoryItemTypes.generated.h"

extern const int32 NAMED_ITEM_SLOT;

struct FARTItemSlot;
class UARTItemStack;
class UGameplayAbility;
class UGameplayEffect;
class UARTItemInputBinder;
class UARTInventoryComponent;
class UARTItemGenerator;
class UARTItemStack_SlotContainer;

//starting item, useful for init inventory
USTRUCT(BlueprintType)
struct ART_API FARTStartingItemEntry
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", Instanced)
	UARTItemGenerator* ItemGenerator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FGameplayTagQuery SlotQuery;
};

//item ability struct
USTRUCT(BlueprintType)
struct ART_API  FARTItemActiveAbilityEntry
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UGameplayAbility> ActiveAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category="Item")
	UARTItemInputBinder* InputBinder;
};

//GAS releated item data
USTRUCT(BlueprintType)
struct ART_API FARTItemDefinition_AbilityInfo
{
	GENERATED_BODY()
public:
	FARTItemDefinition_AbilityInfo()
	{

	}

	//Granted Tags when this item is made active
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TArray<FARTItemActiveAbilityEntry> ActiveAbilityEntries;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TArray<TSubclassOf<UGameplayAbility>> ExtraAbilities;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TArray<TSubclassOf<UGameplayEffect>> AddedGameplayEffects;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TMap<FGameplayAttribute, float> AttributeInitalizers;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Item")
	TArray<TSubclassOf<UAttributeSet>> AttributeSetsToAdd;
};

//for share struct between generator
USTRUCT(BlueprintType)
struct ART_API FARTItemGeneratorContext
{
	GENERATED_BODY();
public:
	FARTItemGeneratorContext()
	{
		Quality = 0;
		UserObject = nullptr;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator Context")
	int32 Quality;

	//An Object set by the user	for use as a reference elsewhere
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator Context")
	UObject* UserObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Generator Context")
	TArray<AActor*> TrackingActors;
};

//containing Tag for slot query
USTRUCT(BlueprintType)
struct ART_API FARTItemSlotFilter
{
	GENERATED_BODY()
public:
	FARTItemSlotFilter()
	{
		ForceSingleStack = false;
	}

	FARTItemSlotFilter(const FARTItemSlotFilter& Other)
		: FilterQuery(Other.FilterQuery),
		ForceSingleStack(Other.ForceSingleStack)
	{

	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Filter)
	FGameplayTagQuery FilterQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Filter)
	bool ForceSingleStack;

	bool AcceptsItem(UARTItemStack* ItemStack) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FARTItemSlotFilter> : public TStructOpsTypeTraitsBase2<FARTItemSlotFilter>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct ART_API FARTItemSlotFilterHandle
{
	GENERATED_BODY()
public:
	FARTItemSlotFilterHandle()
		: Data(nullptr)
	{

	}

	FARTItemSlotFilterHandle(FARTItemSlotFilter* Filter)
		: Data(Filter)
	{

	}	

	bool IsValid() const
	{
		return Data.IsValid();
	}

	bool AcceptsItem(UARTItemStack* ItemStack)	const
	{
		if (!IsValid())
		{
			return false;
		}
		return Data->AcceptsItem(ItemStack);
	}

	TSharedPtr<FARTItemSlotFilter> Data;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FARTItemSlotFilterHandle> : public TStructOpsTypeTraitsBase2<FARTItemSlotFilterHandle>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};

USTRUCT(BlueprintType)
struct ART_API FARTItemQuery
{
	GENERATED_BODY()
public:
	FARTItemQuery()
	{

	}
	  		 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter")
	FGameplayTagQuery ItemTypeQuery;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter")
	FGameplayTagQuery SlotTypeQuery;

	bool MatchesSlot(const FARTItemSlot& ItemSlot) const;

	static FARTItemQuery QuerySlotMatchingTag(FGameplayTag Tag);
	static FARTItemQuery QueryForMatchingItemTag(FGameplayTag Tag);
	
	static FARTItemQuery QueryForSlot(const FGameplayTagQuery& SlotQuery);
	static FARTItemQuery QueryForItemType(const FGameplayTagQuery& ItemQuery);

	bool IsValid() const;
};

USTRUCT(BlueprintType)
struct ART_API FARTEquippedItemInfo
{
	GENERATED_BODY()
public:
	FARTEquippedItemInfo()
	{

	}

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AddedAbilities;
	UPROPERTY()
	TArray<UAttributeSet*> InstancedAttributeSets;
	UPROPERTY()
	TArray< FActiveGameplayEffectHandle> AddedGameplayEffects;
};

USTRUCT(BlueprintType)
struct ART_API FARTItemSlotDefinition
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	FGameplayTagContainer Tags;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	FARTItemSlotFilter Filter;
};

//---------------------------------------------
// INVENTORY ITEM STRUCT
//---------------------------------------------
USTRUCT(BlueprintType)
struct ART_API FARTItemSlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	friend class UARTInventoryComponent;
	friend class UARTItemStack_SlotContainer;
	
public:
	FARTItemSlot()
		:FFastArraySerializerItem(),
		ItemStack(nullptr),
		SlotId(NAMED_ITEM_SLOT),
		OldItemStack(nullptr)
	{
		
	}

	FARTItemSlot(const FARTItemSlot& Copy)
		: FFastArraySerializerItem(Copy),
		ItemStack(Copy.ItemStack),
		ItemSlotFilter(Copy.ItemSlotFilter),
		SlotId(Copy.SlotId),
		SlotTags(Copy.SlotTags),
		OldItemStack(nullptr)
	{

	}

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = Inventory)
	UARTItemStack* ItemStack;

	UPROPERTY()
	FARTItemSlotFilterHandle ItemSlotFilter;	
	
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = Inventory)
	int32 SlotId;	  
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = Inventory)
	FGameplayTagContainer SlotTags;

	TWeakObjectPtr<UARTItemStack> OldItemStack;

	static FARTItemSlot Invalid;

	void ToDebugStrings(TArray<FString>& OutStrings, bool Detailed) const;
	/** Comparison operator */
	bool operator==(FARTItemSlot const& Other) const
	{
		return SlotId == Other.SlotId && ItemStack == Other.ItemStack;
	}

	/** Comparison operator */
	bool operator!=(FARTItemSlot const& Other) const
	{
		return !(FARTItemSlot::operator==(Other));
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	void PreReplicatedRemove(const struct FARTItemSlotArray& InArraySerializer);
	void PostReplicatedAdd(const struct FARTItemSlotArray& InArraySerializer);
	void PostReplicatedChange(const struct FARTItemSlotArray& InArraySerializer);

private:
	UARTInventoryComponent* Owner;
	
	UARTItemStack_SlotContainer* ParentStack;
};

template<>
struct TStructOpsTypeTraits<FARTItemSlot> : public TStructOpsTypeTraitsBase2<FARTItemSlot>
{
	enum
	{
		WithNetSerializer = true,
		WithIdenticalViaEquality = true,
	};
};

USTRUCT()
struct FARTItemSlotArray : public FFastArraySerializer
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TArray<FARTItemSlot> Slots;

	UPROPERTY()
	UARTInventoryComponent* Owner;

	UPROPERTY()
	UARTItemStack_SlotContainer* ParentStack;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FARTItemSlot, FARTItemSlotArray>(Slots, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits< FARTItemSlotArray > : public TStructOpsTypeTraitsBase2< FARTItemSlotArray >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct ART_API FARTItemSlotReference
{
	GENERATED_BODY()
public:

	FARTItemSlotReference(){}
		
	FARTItemSlotReference(const FARTItemSlotReference& Copy);

	FARTItemSlotReference(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory);

	FARTItemSlotReference(const FARTItemSlot& FromSlot, UARTItemStack_SlotContainer* InParentStack);

	FARTItemSlotReference(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory, UARTItemStack_SlotContainer* InParentStack);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Inventory)
	int32 SlotId;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Inventory)
	FGameplayTagContainer SlotTags;
	
	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TWeakObjectPtr<UARTInventoryComponent> ParentInventory;

	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TWeakObjectPtr<UARTItemStack_SlotContainer> ParentStack;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	FString ToString() const;

	static FARTItemSlotReference Invalid;

	// Comparison operator
	bool operator==(const FARTItemSlotReference& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool bParentsMatch = (ParentInventory.Get() == Other.ParentInventory.Get() || ParentStack.Get() == Other.ParentStack.Get());;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bParentsMatch && bIdsMatch && TagsMatch;
	}

	// Comparison operator
	bool operator!=(const FARTItemSlotReference& Other) const
	{
		return !(FARTItemSlotReference::operator==(Other));
	}

	bool operator==(const FARTItemSlot& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bIdsMatch && TagsMatch;
	}

	bool operator!=(const FARTItemSlot& Other) const
	{
		return !(FARTItemSlotReference::operator==(Other));
	}
};

template<>
struct TStructOpsTypeTraits<FARTItemSlotReference> : public TStructOpsTypeTraitsBase2<FARTItemSlotReference>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct ART_API FARTItemSlotRef
{
	GENERATED_BODY()
	
public:
	FARTItemSlotRef(){};
	FARTItemSlotRef(const FARTItemSlotReference& Copy);
	FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory);
	FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTItemStack_SlotContainer* InParentStack);
	FARTItemSlotRef(const FARTItemSlot& FromSlot, UARTInventoryComponent* InParentInventory, UARTItemStack_SlotContainer* InParentStack);


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Inventory)
	int32 SlotId;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Inventory)
	FGameplayTagContainer SlotTags;
	
	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TWeakObjectPtr<UARTInventoryComponent> ParentInventory;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TWeakObjectPtr<UARTItemStack_SlotContainer> ParentStack;

	static FARTItemSlotRef Invalid;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	FString ToString() const;

	// Comparison operator
	bool operator==(const FARTItemSlotRef& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool bParentsMatch = (ParentInventory.Get() == Other.ParentInventory.Get() || ParentStack.Get() == Other.ParentStack.Get());;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bParentsMatch && bIdsMatch && TagsMatch;
	}

	// Comparison operator
	bool operator!=(const FARTItemSlotRef& Other) const
	{
		return !(FARTItemSlotRef::operator==(Other));
	}

	bool operator==(const FARTItemSlot& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bIdsMatch && TagsMatch;
	}

	bool operator!=(const FARTItemSlot& Other) const
	{
		return !(FARTItemSlotRef::operator==(Other));
	}
};

template<>
struct TStructOpsTypeTraits<FARTItemSlotRef> : public TStructOpsTypeTraitsBase2<FARTItemSlotRef>
{
	enum
	{
		WithNetSerializer = true,
	};
};

//---------------------------------------------
// CONTAINER ITEM STRUCT
//---------------------------------------------

/*USTRUCT(BlueprintType)
struct ART_API FARTContainerItemSlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	FARTContainerItemSlot()
		:FFastArraySerializerItem(),
		ItemStack(nullptr),
		SlotId(NAMED_ITEM_SLOT),
		OldItemStack(nullptr)
	{
		
	}

	FARTContainerItemSlot(const FARTContainerItemSlot& Copy)
		: FFastArraySerializerItem(Copy),
		ItemStack(Copy.ItemStack),
		ItemSlotFilter(Copy.ItemSlotFilter),
		SlotId(Copy.SlotId),
		SlotTags(Copy.SlotTags),
		OldItemStack(nullptr)
	{

	}

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = Container)
	UARTItemStack* ItemStack;

	UPROPERTY()
	FARTItemSlotFilterHandle ItemSlotFilter;	
	
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = Container)
	int32 SlotId;	  
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = Container)
	FGameplayTagContainer SlotTags;

	TWeakObjectPtr<UARTItemStack> OldItemStack;

	static FARTContainerItemSlot Invalid;

	void ToDebugStrings(TArray<FString>& OutStrings, bool Detailed) const;
	// Comparison operator 
	bool operator==(FARTContainerItemSlot const& Other) const
	{
		return SlotId == Other.SlotId && ItemStack == Other.ItemStack;
	}

	//Comparison operator
	bool operator!=(FARTContainerItemSlot const& Other) const
	{
		return !(FARTContainerItemSlot::operator==(Other));
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	void PreReplicatedRemove(const struct FARTContainerItemSlotArray& InArraySerializer);
	void PostReplicatedAdd(const struct FARTContainerItemSlotArray& InArraySerializer);
	void PostReplicatedChange(const struct FARTContainerItemSlotArray& InArraySerializer);

private:
	UARTItemStack_SlotContainer* ParentStack;

	friend class UARTItemStack_SlotContainer;
};

template<>
struct TStructOpsTypeTraits<FARTContainerItemSlot> : public TStructOpsTypeTraitsBase2<FARTContainerItemSlot>
{
	enum
	{
		WithNetSerializer = true,
		WithIdenticalViaEquality = true,
	};
};

USTRUCT()
struct FARTContainerItemSlotArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY()
	TArray<FARTContainerItemSlot> Slots;

	UPROPERTY()
	UARTItemStack_SlotContainer* Owner;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FARTContainerItemSlot, FARTContainerItemSlotArray>(Slots, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits< FARTContainerItemSlotArray > : public TStructOpsTypeTraitsBase2< FARTContainerItemSlotArray >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct ART_API FARTContainerItemSlotReference
{
	GENERATED_BODY()
public:

	FARTContainerItemSlotReference()
		: SlotId(NAMED_ITEM_SLOT)
		, ParentStack(nullptr)
	{
		
	}
		
	FARTContainerItemSlotReference(const FARTContainerItemSlotReference& Copy)
		: SlotId(Copy.SlotId),
		SlotTags(Copy.SlotTags),
		ParentStack(Copy.ParentStack)
	{

	}

	FARTContainerItemSlotReference(const FARTContainerItemSlot& FromSlot, UARTItemStack_SlotContainer
	* InParentStack)
		: SlotId(FromSlot.SlotId)
		, SlotTags(FromSlot.SlotTags)
		, ParentStack(InParentStack)
	{

	}

	FARTContainerItemSlotReference(FGameplayTag InTag, UARTItemStack_SlotContainer
	* InParentStack)
		: SlotId(NAMED_ITEM_SLOT)
		, SlotTags(InTag.GetSingleTagContainer())
		, ParentStack(InParentStack)
	{

	}

	FARTContainerItemSlotReference(int32 InSlotId, UARTItemStack_SlotContainer
	* InParentStack)
		: SlotId(InSlotId)
		, ParentStack(InParentStack)
	{

	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Container)
	int32 SlotId;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Container)
	FGameplayTagContainer SlotTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Container)
	UARTItemStack_SlotContainer* ParentStack;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	FString ToString() const;

	static FARTContainerItemSlotReference Invalid;

	// Comparison operator
	bool operator==(const FARTContainerItemSlotReference& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool bParentsMatch = ParentStack == Other.ParentStack;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bParentsMatch && bIdsMatch && TagsMatch;
	}

	// Comparison operator
	bool operator!=(const FARTContainerItemSlotReference& Other) const
	{
		return !(FARTContainerItemSlotReference::operator==(Other));
	}

	bool operator==(const FARTContainerItemSlot& Other) const
	{
		const bool bIdsMatch = SlotId == Other.SlotId;
		const bool TagsMatch = SlotTags.HasAllExact(Other.SlotTags);

		return bIdsMatch && TagsMatch;
	}

	bool operator!=(const FARTContainerItemSlot& Other) const
	{
		return !(FARTContainerItemSlotReference::operator==(Other));
	}
};

template<>
struct TStructOpsTypeTraits<FARTContainerItemSlotReference> : public TStructOpsTypeTraitsBase2<FARTContainerItemSlotReference>
{
	enum
	{
		WithNetSerializer = true,
	};
};*/

//---------------------------------------

ART_API inline bool IsValid(const FARTItemSlotFilterHandle& Handle)
{
	return Handle.IsValid();
}

ART_API bool IsValid(const FARTItemSlotReference& ItemRef);

ART_API bool IsValid(const FARTItemSlotRef& ItemRef);