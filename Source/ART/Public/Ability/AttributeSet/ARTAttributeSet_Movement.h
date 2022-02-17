// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Movement.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Movement : public UARTAttributeSetBase
{
	GENERATED_BODY()

public:
	UARTAttributeSet_Movement();
	
	//MoveSpeed
	UPROPERTY(BlueprintReadOnly, Category = "MoveSpeed", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Movement, MoveSpeed)

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Movement, MoveSpeed, OldMoveSpeed);
	}

	//RotateRate
	UPROPERTY(BlueprintReadOnly, Category = "RotateRate", ReplicatedUsing = OnRep_RotateRate)
	FGameplayAttributeData RotateRate;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Movement, RotateRate)

	UFUNCTION()
	void OnRep_RotateRate(const FGameplayAttributeData& OldRotateRate)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Movement, RotateRate, OldRotateRate);
	}

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute, FAggregator* NewAggregator) const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};