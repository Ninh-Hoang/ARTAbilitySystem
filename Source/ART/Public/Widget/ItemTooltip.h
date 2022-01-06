// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltip.generated.h"

/**
 * 
 */

class UInventoryItemWidget;

UCLASS()
class ART_API UItemTooltip : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "ToolTip", meta = (ExposeOnSpawn = true))
	UInventoryItemWidget* InventoryItemWidget;
};
