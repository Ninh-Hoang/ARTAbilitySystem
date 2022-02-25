// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagAssetInterface.h"
#include "ARTItemStack.generated.h"

/**
 * 
 */
class UARTItemRarity;
class UARTItemStack;
class UARTItemDefinition;
class UARTItemUIData_ItemDefinition;

USTRUCT()
struct FARTSubItemArrayEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
public:
	FARTSubItemArrayEntry()
		: SubItemStack(nullptr)
		, PreviousStack(nullptr)
	{
	}

	UPROPERTY()
	UARTItemStack* SubItemStack;


	TWeakObjectPtr<UARTItemStack> PreviousStack;

	void PreReplicatedRemove(const struct FARTSubItemStackArray& InArraySerializer);
	void PostReplicatedChange(const struct FARTSubItemStackArray& InArraySerializer);
	
};

USTRUCT()
struct FARTSubItemStackArray : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FARTSubItemArrayEntry> Items;

	UARTItemStack* ParentStack;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FARTSubItemArrayEntry, FARTSubItemStackArray>(Items, DeltaParms, *this);
	}

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FARTItemStackSubstackEvent, UARTItemStack*, ItemStack, UARTItemStack*, SubItemStack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTItemStackStackSizeEvent, UARTItemStack*, ItemStack, int32, OldStackSize, int32, NewStackSize);

UCLASS(Blueprintable, BlueprintType)
class ART_API UARTItemStack : public UObject, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	friend class UARTItemGenerator;
	
public:
	UARTItemStack(const FObjectInitializer& ObjectInitializer);
	
	static void TransferStackOwnership(UARTItemStack*& ItemStack, AActor* Owner);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags);
	
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	AActor* GetOwner() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item", ReplicatedUsing=OnRep_Rarity)
	UARTItemRarity* Rarity;	

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="UI", ReplicatedUsing=OnRep_ItemName)
	FText ItemName;

	UPROPERTY(BlueprintAssignable)
	FARTItemStackSubstackEvent OnSubStackAdded;
	
	UPROPERTY(BlueprintAssignable)
	FARTItemStackSubstackEvent OnSubStackRemoved;

	UPROPERTY(BlueprintAssignable)
	FARTItemStackStackSizeEvent OnStackSizeChanged;
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item", ReplicatedUsing=OnRep_ItemDefinition)
	TSubclassOf<UARTItemDefinition> ItemDefinition;
	
	UPROPERTY(Replicated)
	FARTSubItemStackArray SubItemStacks;

	UPROPERTY(BlueprintReadOnly)
	UARTItemStack* ParentItemStack;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item", ReplicatedUsing=OnRep_StackSize)
	int32 StackSize;

public:

	void SetParentStack(UARTItemStack* ItemStack);
	UARTItemStack* GetParentStack() const {return ParentItemStack; }
	
	UFUNCTION()
	virtual void OnRep_Rarity(UARTItemRarity* PreviousRarity);
	UFUNCTION()
	virtual void OnRep_ItemName(FText PreviousItemName);
	UFUNCTION()
	virtual void OnRep_ItemDefinition(TSubclassOf<UARTItemDefinition> PreviousItemDefinition);
	UFUNCTION()
	virtual void OnRep_StackSize(int32 PreviousStackSize);

	virtual TSubclassOf<UARTItemDefinition> GetItemDefinition() const;
	virtual int32 GetStackSize() const;

	UFUNCTION(BlueprintPure, Category = "ART|Inventory|ItemStack")
	virtual bool CanStackWith(UARTItemStack* OtherStack) const;

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|ItemStack")
	virtual void SetStackSize(int32 NewStackSize);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory|ItemStack")
	virtual bool HasValidItemData() const;

	//Returns true if merged completely.  False if OtherStack has elements remaining.   
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|ItemStack")
	virtual bool MergeItemStacks(UARTItemStack* OtherStack);

	//Returns true if the item split successfully and OutRemaining has valid data
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|ItemStack")
	virtual UARTItemStack* SplitItemStack(int32 SplitAmount);	

	UFUNCTION(BlueprintNativeEvent)
	bool CanAttachTo(UARTItemStack* OtherStack);
	virtual bool CanAttachTo_Implementation(UARTItemStack* OtherStack);

	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|ItemStack")
	virtual bool AddSubItemStack(UARTItemStack* SubItemStack);
	UFUNCTION(BlueprintCallable, Category = "ART|Inventory|ItemStack")
	virtual bool RemoveSubItemStack(UARTItemStack* SubItemStack);

	UFUNCTION(BlueprintPure, Category="ART|Inventory|ItemStack")
	virtual UARTItemUIData_ItemDefinition* GetUIData();

	UFUNCTION(BlueprintPure, Category = "ART|Inventory|ItemStack")
	virtual UARTItemStack* QueryForSubItem(const FGameplayTagQuery& StackQuery);

	UFUNCTION(BlueprintPure, Category = "ART|Inventory|ItemStack")
	virtual void GetSubItems(TArray<UARTItemStack*>& SubItemArray);
	
	virtual void GetDebugStrings(TArray<FString>& OutStrings, bool Detailed) const;
};
