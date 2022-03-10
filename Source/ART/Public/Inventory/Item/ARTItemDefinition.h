#pragma once

#include "CoreMinimal.h"
#include "Inventory/UI/ARTItemUIData.h"
#include "GameplayTagAssetInterface.h"
#include "ARTItemDefinition.generated.h"

class UARTItemStack;
class UStaticMesh;

UCLASS(Blueprintable, BlueprintType, Abstract)
class ART_API UARTItemDefinition : public UObject, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:
	UARTItemDefinition();

	//Max stack size of this item, if it can stack
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "ItemData", Meta =(ClampMin=1, ClampMax = 999))
	int32 MaxStackSize;

	//Max stack size of this item, if it can stack
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "ItemData", Meta =(ClampMin=0))
	int32 ItemPrice;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "ItemData")
	TSubclassOf<UARTItemStack> DefaultItemStackClass;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Display")
	UStaticMesh* WorldItemModel;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadWrite, Category = "UI", Meta = (AssetBundles = "UI"))
	UARTItemUIData_ItemDefinition* UIData;

	//Tags that describe this team
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer OwnedTags;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AppendTags(OwnedTags);
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};