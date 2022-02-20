#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ARTItemUIData.generated.h"

class UARTInventoryBaseItemCardWidget;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, Abstract)
class ART_API UARTItemUIData : public UObject
{
	GENERATED_BODY()
public:
	UARTItemUIData();
};

UCLASS(Blueprintable, Abstract, BlueprintType)
class ART_API UARTItemUIData_ItemDefinition : public UARTItemUIData 
{
	GENERATED_BODY()
public:
	//TODO: add more data here
	UARTItemUIData_ItemDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UARTInventoryBaseItemCardWidget> SmallItemCard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UARTInventoryBaseItemCardWidget> ItemTooltip;
};