// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ConsumableItem.generated.h"

/**
 * 
 */

UCLASS()
class ART_API UConsumableItem : public UItem
{
	GENERATED_BODY()

public:
	UConsumableItem();

protected:
	virtual void Use(class AARTCharacterBase* Character) override;
};
