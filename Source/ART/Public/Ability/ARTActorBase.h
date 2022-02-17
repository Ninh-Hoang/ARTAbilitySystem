// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include <GenericTeamAgentInterface.h>
#include "ARTActorBase.generated.h"

class UARTAttributeSetBase;
class UARTAbilitySet;

UCLASS()
class ART_API AARTActorBase : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AARTActorBase();

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	class UARTAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	TArray<TSoftObjectPtr<UARTAbilitySet>> AbilitySets;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void CreateASC();
	virtual void InitializeASC();
};
