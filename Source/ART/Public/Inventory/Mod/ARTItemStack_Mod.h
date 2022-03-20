// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTItemModTypes.h"
#include "Inventory/ARTItemStack.h"
#include "ARTItemStack_Mod.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class ART_API UARTItemStack_Mod : public UARTItemStack
{
	GENERATED_BODY()
public:
	UARTItemStack_Mod(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mod Settings", Replicated)
	TArray<FARTModModifierData> DynamicModifiers;
	   
	virtual bool CanAttachTo_Implementation(UARTItemStack* OtherStack) override;
};
