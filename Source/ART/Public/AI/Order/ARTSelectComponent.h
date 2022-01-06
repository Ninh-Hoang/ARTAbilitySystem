// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTSelectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FONSelectEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ART_API UARTSelectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UARTSelectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Order")
	FONSelectEvent OnSelected;

	UPROPERTY(BlueprintAssignable, Category = "Order")
	FONSelectEvent OnDeselected;

	UFUNCTION(BlueprintPure, Category=Selectable)
	bool IsSelected();

	UFUNCTION(BlueprintCallable, Category=Selectable)
	bool SetSelected(bool Selected);
private:
	bool bSelected;

	bool InternalSetSelected(bool Selected);
};
