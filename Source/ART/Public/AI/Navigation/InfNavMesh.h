// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	const TMap<FIntVector, float>* InfluenceMap;

	const float FIXED_ADDITIONAL_COST;
	float CostMultiplier;

	// �f�o�b�O�p.
	class UWorld* World;
	bool bDrawDebugFindPath;

public:
	FInfQueryFilter(bool inIsVirtual = true);
	virtual ~FInfQueryFilter() {}

	void SetDataUsedForDebugging(class UWorld* NewWorld, bool bDrawDebugEnabled);
	void SetAdditionalNavigationData(const TMap<FIntVector, float>& InInfluenceMapData, float InCostMultiplier);

	virtual void Reset() override;
	virtual bool IsEqual(const INavigationQueryFilterInterface* Other) const override;
	const FInfQueryFilter* GetAsDetourQueryFilter() const { return this; }

protected:
	// �o�H�Ԃ̈ړ��R�X�g�����ۂɌv�Z����.
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
	
private:
	FInfQueryFilter* DetourFilter;
	
public:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void RecreateDefaultFilter() override;

	void SetInfluenceMapData(const struct FAIInfMapMoveRequest& InfluenceMapData);

	virtual void OnNavMeshTilesUpdated(const TArray<uint32>& ChangedTiles) override;
};
