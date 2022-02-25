

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagAssetInterface.h" 
#include "ARTItemRarity.generated.h"


UCLASS(Blueprintable, BlueprintType, EditInlineNew, Abstract)
class ART_API UARTItemRarity_UserData : public UObject
{
	GENERATED_BODY()
public:

};

/**
 * 
 */
UCLASS(BlueprintType)
class ART_API UARTItemRarity : public UPrimaryDataAsset, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:

	UARTItemRarity(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FGameplayTagContainer RarityTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FGameplayTagContainer TagsGrantedToItem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FText RarityName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FLinearColor RarityColor;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category= "Rarity")
	UARTItemRarity_UserData* UserData;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
	{
		TagContainer.AppendTags(RarityTags);
	}
};