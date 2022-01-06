// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARTOrderPreview.generated.h"

class UARTGameplayAbility;

UCLASS()
class ART_API AARTOrderPreview : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AARTOrderPreview();

	UFUNCTION(Category = "Order|Preview", BlueprintPure)
	UARTGameplayAbility* GetInstigatorAbility() const;

	UFUNCTION(Category = "Order|Preview", BlueprintCallable)
	void SetInstigatorAbility(UARTGameplayAbility* Ability);

	UFUNCTION(Category = "Order|Preview", BlueprintImplementableEvent, meta = (DisplayName = "OnInstigatorAbilityInitialized"))
	void ReceiveOnInstigatorAbilityInitialized();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** InstigatorAbility will be used to get information, like the range of an ability, for the OrderPreview */
	UPROPERTY(Category = "Order|Preview", BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = true))
	UARTGameplayAbility* InstigatorAbility;
};
