// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ART_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItem;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0.0))
	float WeightCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0))
	int32 Capacity;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<UItem*> Items;

	//delegate
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	                                 FReplicationFlags* RepFlags) override;

public:
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(UItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<UItem> ItemClass, const int32 Quantity);

	int32 ConsumeItem(UItem* Item);
	int32 ConsumeItem(UItem* Item, const int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItem* Item);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<UItem> ItemClass, const int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItem(UItem* Item) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItemByClass(TSubclassOf<UItem> ItemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UItem*> FindItemsByClass(TSubclassOf<UItem> ItemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<UItem*> GetItems() const { return Items; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

private:
	UPROPERTY()
	int32 ReplicatedItemsKey;

	//handle replication and ownership
	UItem* AddItem(UItem* Item);

	//non-BP internal function
	FItemAddResult TryAddItem_Internal(UItem* Item);

	UFUNCTION()
	void OnRep_Items();
};
