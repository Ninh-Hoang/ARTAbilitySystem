// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InfMapFunctionLibrary.h"
#include "Navigation/RecastGraphAStar.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastQueryFilter.h"
#include "InfNavMesh.generated.h"

/**
 * 
 */
class ART_API FInfQueryFilter : public FRecastQueryFilter
{
private:
	TMap<FIntVector, float> InfluenceMap;

	const float FIXED_ADDITIONAL_COST;
	float CostMultiplier;
	
	class UWorld* World;
	bool bDrawDebugFindPath;

public:
	FInfQueryFilter(bool inIsVirtual = true);
	virtual ~FInfQueryFilter() {}

	void SetDataUsedForDebugging(class UWorld* NewWorld, bool bDrawDebugEnabled);
	void SetAdditionalNavigationData(const FInfQueryData& InInfluenceQueryData, float InCostMultiplier);

	virtual void Reset() override;
	virtual bool IsEqual(const INavigationQueryFilterInterface* Other) const override;
	const FInfQueryFilter* GetAsDetourQueryFilter() const { return this; }

protected:

	virtual float getVirtualCost(const float* pa, const float* pb,
		const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
		const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly,
		const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const override;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedToUpdateGraph, const TArray<uint32>&, ChangedTiles);

UCLASS()
class ART_API AInfNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere)
	bool bDrawDebugFindPath;

	FOnNeedToUpdateGraph OnNeedToUpdateGraph;

protected:
	UPROPERTY()
	TScriptInterface<class IInfCollectionInterface> InfluenceMapCollectionInterfaceRef;

	class IInfCollectionInterface* InfluenceMapCollectionRef;
	
private:
	FInfQueryFilter* DetourFilter;

	float NavigationTickTime = 1.f;
public:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void RecreateDefaultFilter() override;

	void SetInfluenceQueryData(const struct FAIInfMapMoveRequest& InfluenceMapData);

	virtual void OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles) override;

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
};
