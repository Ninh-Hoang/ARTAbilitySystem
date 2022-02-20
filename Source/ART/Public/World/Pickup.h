// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class UActorChannel;
class UStaticMeshComponent;

UCLASS()
class ART_API APickup : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickup();
	
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	UStaticMeshComponent* PickupMesh;

	UFUNCTION()
	void OnTakePickup(class AARTSurvivor* Taker);
	

	//refresh UI/Notification
	UFUNCTION()
	void OnItemModified();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
