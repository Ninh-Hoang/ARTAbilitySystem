// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARTItemStackWorldObject.generated.h"

class UARTItemStack;

UCLASS()
class ART_API AARTItemStackWorldObject : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMeshComponent;
	
public:	
	// Sets default values for this actor's properties
	AARTItemStackWorldObject(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

	
	UStaticMeshComponent* GetStaticMeshComponent() { return StaticMeshComponent; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Item")
	void SetInventoryStack(UARTItemStack* InInventoryStack);
	virtual void SetInventoryStack_Implementation(UARTItemStack* InInventoryStack);

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual UARTItemStack* GetInventoryStack();



	

	UFUNCTION()
	void OnRep_InventoryStack();

	//virtual void OnInteract_Implementation(AActor* Interactor, bool bLongHold) override;

	private:
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_InventoryStack, Category = "Item")
	UARTItemStack* InventoryStack;

};
