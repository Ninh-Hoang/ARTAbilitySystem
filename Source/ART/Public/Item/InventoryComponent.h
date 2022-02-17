// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Engine/NetSerialization.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UInventoryComponent;
class UItem;

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemAdded UMETA(DisplayName = "No items added"),
	IAR_SomeItemAdded UMETA(DisplayName = "Some items added"),
	IAR_AllItemAdded UMETA(DisplayName = "All items added")
};

USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

public:

	FItemAddResult()
	{
	};

	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), ActualAmountGiven(0)
	{
	};

	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity),
	                                                              ActualAmountGiven(InQuantityAdded)
	{
	};

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountGiven;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText;

	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNonResult(InItemQuantity);
		AddedNonResult.Result = EItemAddResult::IAR_NoItemAdded;
		AddedNonResult.ErrorText = ErrorText;
		return AddedNonResult;
	}

	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);
		AddedSomeResult.Result = EItemAddResult::IAR_SomeItemAdded;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddedAllResult(InItemQuantity, InItemQuantity);
		AddedAllResult.Result = EItemAddResult::IAR_AllItemAdded;

		return AddedAllResult;
	}
};

USTRUCT(BlueprintType)
struct ART_API FItemData : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FItemData()
		:Item(nullptr),
		Quantity(0){}
	
	FItemData(UItem* InItem)
		:Item(InItem),
		Quantity(1){}

	FItemData(UItem* InItem, int32 InQuantity)
		:Item(InItem),
		Quantity(InQuantity)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	UItem* Item;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	int32 Quantity;
	
	bool operator=(const FItemData& Other) const {return (Item == Other.Item) && (Quantity == Other.Quantity);}
	bool IsValid() const { return Item != nullptr && Quantity >= 1; }
	
	UItem* GetItem() const {return Item;} 
	int32 GetQuantity() const  {return Quantity;}
	void AddQuantity(int32 AddAmount) { Quantity = FMath::Clamp(Quantity + AddAmount, 0, Item->MaxStackSize);}
	void SetQuantity(int32 InQuantity) {Quantity = FMath::Clamp(InQuantity, 0, Item->MaxStackSize);}

	bool ShouldReplicateItemData() const;
	
	void PostReplicatedAdd(const struct FItemDataContainer &InArray);
	void PreReplicatedRemove(const struct FItemDataContainer &InArray);
	void PostReplicatedChange(const struct FItemDataContainer &InArray);

	FString GetDebugString();
};

USTRUCT(BlueprintType)
struct ART_API FItemDataContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FItemDataContainer()
	: Owner(nullptr){}

	UPROPERTY(BlueprintReadWrite)
	TArray<FItemData> Items;

	UPROPERTY()
	UInventoryComponent* Owner;

	void RegisterOwner(UInventoryComponent* Owner);

	int32 Num() const {return Items.Num();}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemData, FItemDataContainer>(Items, DeltaParms, *this);
	}
	
	template< typename Type, typename SerializerType >
	bool ShouldWriteFastArrayItem(const Type& Item, const bool bIsWritingOnClient)
	{
		// if we do not want the Item to replicated return false;
		if (!Item.ShouldReplicateItemData())
		{
			return false;
		}

		if (bIsWritingOnClient)
		{
			return Item.ReplicationID != INDEX_NONE;
		}

		return true;
	}
};

template<>
struct TStructOpsTypeTraits< FItemDataContainer > : public TStructOpsTypeTraitsBase2< FItemDataContainer >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

// Used to stop us from removing items from an inventory while we're iterating through the items
struct ART_API FScopedItemListLock
{
	FScopedItemListLock(UInventoryComponent& InContainer);
	~FScopedItemListLock();

private:
	UInventoryComponent& InventoryComponent;
};

#define ITEMLIST_SCOPE_LOCK()	FScopedItemListLock ActiveScopeLock(*this);

//Used to stop us from removing an item while we're iterating through its ability/effect
struct ART_API FScopedItemAbilityListLock
{
	FScopedItemAbilityListLock(UInventoryComponent& InInventoryComponent, const UItem& InItem);
	~FScopedItemAbilityListLock();

private:
	const UItem& Item;

	// we also need to make sure item isn't removed while we're in this lock
	FScopedItemListLock ItemLock;
};

#define TARGETLIST_SCOPE_LOCK(Inventory)	FScopedTargetListLock ActiveScopeLock(Inventory, *this);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ART_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItem;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	FItemDataContainer ItemContainer;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0))
	int32 Capacity;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0.0))
	float WeightCapacity;

	//delegate
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY()
	bool bCachedIsNetSimulated;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UInventoryComponent();

	virtual void OnRegister() override;
	void CacheIsNetSimulated();

	virtual void OnAddItem(FItemData& ItemData);
	virtual void OnRemoveItem(FItemData& ItemData);
	virtual void OnItemDataChanged(FItemData& ItemData);

	int32 ItemScopeLockCount;
	TArray<FItemData, TInlineAllocator<2> > ItemPendingAdds;
	TArray<FItemData, TInlineAllocator<2> > ItemPendingRemoves;

	void IncrementItemListLock();
	void DecrementItemListLock();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(const UItem* Item, int32 Quantity);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemData(const FItemData& Item);

	/*UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<UItem> ItemClass, const int32 Quantity);

	int32 ConsumeItem(UItem* Item);
	int32 ConsumeItem(UItem* Item, const int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FItemData& Item);*/

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(const UItem* SearchItem, const int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindItem(const UItem* SearchItem, FItemData& OutItem) const;
	
	bool FindItems(const UItem* SearchItem, TArray<FItemData*>& OutItems) const;

	bool FindAllItemWithNotMaxStack(const TSubclassOf<UItem> ItemClass, TArray<FItemData*>& OutItems) const;
	

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<FItemData>& GetItemList() const { return const_cast<TArray<FItemData>&>(ItemContainer.Items); }
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

private:

	//handle replication and ownership
	FItemData* AddItem(UItem* Item);

	FItemData* AddItem(UItem* Item, int32 Quantity);

	UFUNCTION()
	void OnRep_Items();
};
