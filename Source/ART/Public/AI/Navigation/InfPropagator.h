// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "InfPropagatorInterface.h"
#include "InfStruct.h"
#include "Components/ActorComponent.h"
#include "InfPropagator.generated.h"

using PropagationValueCalculator = TFunctionRef<float(uint32, uint32, const FVector&)>;
using ExcludeFromPropagationValueCalc = TFunctionRef<bool(const AActor*)>;

struct FPropagationMap
{
	TMap<FIntVector, float> MapData;
	float MaxValue;

	FPropagationMap()
		: MapData()
		, MaxValue(-1.f)
	{}

	FPropagationMap(TMap<FIntVector, float> NewMapData, float NewMaxValue)
		: MapData(NewMapData)
		, MaxValue(NewMaxValue)
	{}
};

UCLASS(meta=(BlueprintSpawnableComponent))
class ART_API UInfPropagator : public UActorComponent, public IInfPropagatorInterface//, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInfPropagator();

	UPROPERTY(EditAnywhere, Category = "Influence Map | Base")
	FGameplayTag TargetMapTag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Influence Map | Base", meta = (ClampMin = 1.00))
	bool bActivate;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Influence Map | Base", meta = (ClampMin = 1.00))
	float PropagateRange;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Influence Map | Base", meta = (ClampMax = 1.00, ClampMin = 0.01))
	float AttenuationRatio;
	
	UPROPERTY(EditAnywhere, Category = "Influence Map | Base")
	TArray<FGenericTeamId> AllyTeam;
	
	UPROPERTY(EditAnywhere, Category = "Influence Map | Base")
	TArray<FGenericTeamId> EnemyTeam;
	
	UPROPERTY(EditAnywhere, Category = "Influence Map | Base")
	class UCurveFloat* PropagationCurve;

protected:
	UPROPERTY()
	TScriptInterface<class IInfCollectionInterface> InfluenceMapCollectionInterfaceRef;

	class IInfCollectionInterface* InfluenceMapCollectionRef;
	
	FTransform PreviousPropagatorTransform;

	const float FarawayDistanceToleranceSq = FMath::Square(300.f);

	TArray<FPropagationMap> PropagationMapHistory;
	TMap<FIntVector, float> RecentPropagationMap;
	TMap<FIntVector, float> MergedPropagationMap;
	TMap<FIntVector, float> RecentInterestMap;

	FVector OwnerHeadOffset, OwnerFeetOffset;
	
protected:
    // Called when the game starts
    virtual void BeginPlay() override;
	
public:
	virtual void Initialize(class IInfCollectionInterface* InfluenceMapCollection) override;
	virtual void UpdatePropagationMap() override;
	
protected:
	virtual TMap<FIntVector, float> CreateNewPropagationMap(const FInfNode* CenterNode) const;
	virtual bool ShouldUpdatePropagationMap() const;
	bool FarAwayFrom(const FVector& Location) const;

private:
	void AttenuationPropagationMaps();
	void MergePropagationMaps();

public:
	virtual FGameplayTag GetTargetMapTag() const override { return TargetMapTag; }
	
	// Assigns Team Agent to given TeamID 
	/*virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	
	// Retrieve team identifier in form of FGenericTeamId 
	virtual FGenericTeamId GetGenericTeamId() const override;

	// Retrieved owner attitude toward given Other object
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;*/

	virtual const AActor* GetOwnerActor() const override { return GetOwner(); }

	virtual FGenericTeamId GetTeam() const override;

protected:
	TMap<FIntVector, float> CreateNewMap(const FInfNode* CenterNode, float MaxRange, PropagationValueCalculator PropagationValueFunc, ExcludeFromPropagationValueCalc ExcludeFunc) const;

public:
	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	void SetPropagationActivation(bool Activate);
	
	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	virtual const TMap<FIntVector, float>& GetPropagationMap() const override { return MergedPropagationMap; }

	UFUNCTION(BlueprintCallable, Category = "Influence Map")
	const TMap<FIntVector, float>& CreateInterestMap(float InterestMapRadius, const FVector& Center, class UCurveFloat* InterestCurve = nullptr, float InitializeValue = 0.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
		FVector GetGraphLocationToWorld(const FIntVector& GraphLocation);

	// GatherDistance
	// Infinite Range : <= 0
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	TMap<FIntVector, float> GetEnemyMap(const FGameplayTag MapTag, float GatherDistance) const;
	
	// GatherDistance
	// Infinite Range : <= 0
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Influence Map")
	TMap<FIntVector, float> GetAllyMap(const FGameplayTag MapTag, bool bIgnoreSelf = true, float GatherDistance = 0.f) const;
	UFUNCTION(BlueprintCallable, Category = "Influence Map | DEBUG")
	void DrawDebugPropagationMap(const FGameplayTag MapTag, float Duration) const;
	UFUNCTION(BlueprintCallable, Category = "Influence Map | DEBUG")
	void DrawDebugWorkingMap(const TMap<FIntVector, float>& Map, float Duration);
	

};
