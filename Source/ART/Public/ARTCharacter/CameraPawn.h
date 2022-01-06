// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

class AARTCharacterAI;

UCLASS()
class ART_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

protected:
	UPROPERTY()
	TArray<AARTCharacterAI*> SelectedUnits;
	
	UPROPERTY()
	AARTCharacterAI* UnitA;

	UPROPERTY()
	AARTCharacterAI* UnitB;

	UFUNCTION(BlueprintPure, Category="Team")
	TArray<AARTCharacterAI*> GetSelectedUnit() {return SelectedUnits;}
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void MoveForward(float AxisValue);

	virtual void MoveRight(float AxisValue);

	virtual void ChangeToUnitA();
	virtual void ChangeToUnitB();

	virtual void PressQ();
	virtual void ReleaseQ();

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
	TSubclassOf<AARTCharacterAI> UnitClassA;

	UPROPERTY(EditAnywhere, Category = "Team")
	TSubclassOf<AARTCharacterAI> UnitClassB;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category="Team")
	void BP_ChangeCurrentUnit(int32 UnitIndex, bool& Success);

private:
	void InitSpawnPlayerTeam();
	bool ChangeCurrentUnitInternal(AARTCharacterAI* Unit);

	bool SelectUnit(AARTCharacterAI* Unit);
	bool DeSelectUnit(AARTCharacterAI* Unit);
};
