// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ARTGameplayAbilitySet.h"
#include "GameFramework/Pawn.h"
#include "Inventory/ARTInventoryItemTypes.h"
#include "Inventory/Interfaces/ARTInventoryInterface.h"
#include "CameraPawn.generated.h"

class AARTCharacterAI;

UCLASS()
class ART_API ACameraPawn : public APawn, public IAbilitySystemInterface, public IARTInventoryInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TArray<FARTStartingItemEntry> StartingItems;

	virtual void PossessedBy(AController* NewController) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UARTAbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTInventoryComponent_Active* InventoryComponent;

	virtual void OnRep_PlayerState() override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void MoveForward(float AxisValue);

	virtual void MoveRight(float AxisValue);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UARTInventoryComponent* GetInventoryComponent() const override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Abilities & Atrtibutes")
	TArray<TSoftObjectPtr<UARTAbilitySet>> AbilitySets;

	TArray<FARTAbilitySetHandle> AbilitySetHandles;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Abilities & Atrtibutes")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	UPROPERTY(EditAnywhere, Category = "ART|Delegate")
	FGameplayTagBlueprintPropertyMap TagDelegateMap;

	//Tag Response Table
	UPROPERTY(EditAnywhere, Category = "ART|Delegate")
	class UGameplayTagReponseTable* TagReponseTable;

	virtual void InitializeAbilitySet();
	virtual void InitializeAttributes();
	virtual void InitializeTagPropertyMap();
	virtual void InitializeTagResponseTable();

	UPROPERTY()
	TArray<AARTCharacterAI*> SelectedUnits;
	
	UPROPERTY()
	AARTCharacterAI* PlayerPawn;

	UFUNCTION(BlueprintPure, Category="Team")
	TArray<AARTCharacterAI*> GetSelectedUnit() {return SelectedUnits;}
	
public:
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* AzimuthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UFloatingPawnMovement* MovementComponent;

	UPROPERTY()
	class UInputComponent* DummyInputComp;

	UPROPERTY(EditAnywhere, Category = "Team")
	TSubclassOf<AARTCharacterAI> PlayerPawnClass;

	//TODO: This should change to be a data structure
	UPROPERTY(EditAnywhere, Category = "Team")
	TArray<TSubclassOf<AARTCharacterAI>> AlliesPawnClassList;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category="Team")
	void BP_ChangeCurrentUnit(AARTCharacterAI* Pawn, bool& Success);
	
	UFUNCTION(BlueprintPure, Category="Team")
	AARTCharacterAI* BP_GetControlledUnit() const;
	
private:
	void InitSpawnPlayerTeam();
	bool ChangeCurrentUnitInternal(AARTCharacterAI* Unit);

	bool SelectUnit(AARTCharacterAI* Unit);
	bool DeSelectUnit(AARTCharacterAI* Unit);
};
