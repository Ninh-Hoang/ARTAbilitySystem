// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfCollectionInterface.h"
#include "GameFramework/Actor.h"
#include "InfMapCollection.generated.h"

UCLASS()
class ART_API AInfMapCollection : public AActor, public IInfCollectionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInfMapCollection();

	UPROPERTY(EditAnywhere, Category = "Influence Map")
	TArray<FGameplayTag> ConstructInfluenceMapData;

	UPROPERTY(EditAnywhere, Category = "Influence Map")
	bool bConstructInfluenceMaps;
	
private:
	//cache list of graph interface and map interface
	UPROPERTY()
	TScriptInterface<class IInfGraphInterface> NodeGraphInterface;
	
	UPROPERTY()
	TMap<FGameplayTag, TScriptInterface<class IInfMapInterface>> IMapInterfaceCollection;
	
	const class IInfGraphInterface* NodeGraphRef;
	TMap<FGameplayTag, class IInfMapInterface*> Collection;

public:
	virtual class IInfMapInterface* GetMapSafe(const FGameplayTag& MapTag) const override;
	virtual const class IInfGraphInterface* GetNodeGraph() const override { check(NodeGraphRef); return NodeGraphRef; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void PostLoad() override;

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
