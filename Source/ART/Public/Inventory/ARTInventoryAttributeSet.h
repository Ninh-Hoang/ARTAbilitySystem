// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "ARTInventoryAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTInventoryAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UARTInventoryAttributeSet(const class FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_InventorySize)
	FGameplayAttributeData InventorySize;

	UFUNCTION()
	void OnRep_InventorySize(FGameplayAttributeData OldInventorySize);

	static FGameplayAttribute InventorySizeAttribute();
};
