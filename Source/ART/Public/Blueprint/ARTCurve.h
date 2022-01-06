// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Curves/RichCurve.h"
#include "Curves/CurveBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <CurviestCurve.h>
#include <GameplayTagContainer.h>
#include "ARTCurve.generated.h"

UCLASS()
class ART_API UARTCurveBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Curves", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	//static float GetValueFromCurve(UCurveBase* Curve, FName Name, float InTime);
};

USTRUCT(BlueprintType)
struct FARTCurveData : public FCurviestCurveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "ARTCurviest")
	FGameplayTag CurveTag;

	FARTCurveData()
	{
		this->Color = FLinearColor::White;
	}

	FARTCurveData(FName Name, FLinearColor Color)
	{
		this->Name = Name;
		this->Color = Color;
	}

public:
};

UCLASS(BlueprintType, collapsecategories, hidecategories = (Curviest,FilePath))
class ART_API UARTCurve : public UCurveCurviest
{
	GENERATED_BODY()

public:

	UARTCurve();
	~UARTCurve();

	/*UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	TArray<FName> GetCurveNames() const {
		return CurveNames.Array();
	}*/

	/** Evaluate this float curve at the specified time */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	float GetCurveValueByName(FName Name, float InTime) const;

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	float GetCurveValueByTag(FGameplayTag QueryTag, float InTime) const;

	virtual bool GetCurveTagList(FGameplayTagContainer &TagList);

	// Begin FCurveOwnerInterface
	virtual TArray<FRichCurveEditInfoConst> GetCurves() const override;
	virtual TArray<FRichCurveEditInfo> GetCurves() override;

	/** @return Color for this curve */
	virtual FLinearColor GetCurveColor(FRichCurveEditInfo CurveInfo) const override;

	virtual bool IsValidCurve(FRichCurveEditInfo CurveInfo) override;

	/** Determine if Curve is the same */
	bool operator ==(const UARTCurve& Curve) const;

#if WITH_EDITOR
	void MakeCurveNameUnique(int CurveIdx);

	virtual void PreEditChange(class FEditPropertyChain& PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedChainEvent) override;
	//virtual void OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos) override;
#endif

	UPROPERTY(EditAnywhere, Category = "ARTCurviest")
	UARTCurve *ParentCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "ARTCurviest")
	TArray<FARTCurveData> ARTCurveData;

	virtual void RebuildLookupMaps() override;
};
