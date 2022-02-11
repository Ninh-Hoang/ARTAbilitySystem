// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTCharacter/ARTCharacterBase.h"
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

	//Group AI
protected:
	int32 GroupIndex;

public:
	UFUNCTION(BlueprintPure)
	int32 GetGroupIndex();

	UFUNCTION(BlueprintPure)
	bool IsInGroup();

	void SetGroupIndex(int32 InIndex);
	void RemoveFromGroup();
	
};
