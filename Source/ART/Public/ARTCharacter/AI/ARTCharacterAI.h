// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "ARTCharacter/ARTCharacterAttributeSet.h"

#include "ARTCharacterAI.generated.h"


/**
 * 
 */
UCLASS()
class ART_API AARTCharacterAI : public AARTCharacterBase
{
	GENERATED_BODY()

public:
	//INITIALIZATION
	AARTCharacterAI(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void FinishDying() override;

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Abilities")
	class TSubclassOf<UARTAttributeSetBase> AttributeSetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTNavigationInvokerComponent* NavInvoker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTSelectComponent* SelectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTOrderComponent* OrderComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTAutoOrderComponent* AutoOrderComponent;

	FDelegateHandle HealthChangedDelegateHandle;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
