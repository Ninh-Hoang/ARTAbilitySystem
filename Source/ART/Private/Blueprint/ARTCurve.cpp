// Fill out your copyright notice in the Description page of Project Settings.

#include "Blueprint/ARTCurve.h"

static FName NAME_CurveDefault(TEXT("Curve_0"));

UARTCurve::UARTCurve()
{
	ARTCurveData.Add(FARTCurveData(NAME_CurveDefault, FLinearColor::MakeRandomColor()));
	bLookupsNeedRebuild = true;
}

UARTCurve::~UARTCurve()
{
}

void UARTCurve::RebuildLookupMaps()
{
	if (bLookupsNeedRebuild)
	{
		CurveLookupByName.Reset();
		CurveLookupByTag.Reset();
		for (int i = 0; i < ARTCurveData.Num(); i++)
		{
			auto &Data = ARTCurveData[i];
			CurveLookupByName.Add(Data.Name, i);
			CurveLookupByTag.Add(Data.CurveTag, i);
		}
		bLookupsNeedRebuild = false;
	}
}

float UARTCurve::GetCurveValueByName(FName Name, float InTime) const
{
	const_cast<UARTCurve*>(this)->RebuildLookupMaps();

	const int *CurveIdx = CurveLookupByName.Find(Name);
	if (CurveIdx)
	{
		return ARTCurveData[*CurveIdx].Curve.Eval(InTime);
	}

	return 0.f;
}

float UARTCurve::GetCurveValueByTag(FGameplayTag QueryTag, float InTime) const
{
	const_cast<UARTCurve*>(this)->RebuildLookupMaps();

	const int *CurveIdx = CurveLookupByTag.Find(QueryTag);
	if (CurveIdx)
	{
		return ARTCurveData[*CurveIdx].Curve.Eval(InTime);
	}

	// Check for parent data
	if (ParentCurve)
		return ParentCurve->GetCurveValueByTag(QueryTag, InTime);

	return 0.f;
}

bool UARTCurve::GetCurveTagList(FGameplayTagContainer &TagList)
{
	for (const auto& Data : ARTCurveData)
		TagList.AddTag(Data.CurveTag);

	if(ParentCurve) ParentCurve->GetCurveTagList(TagList);

	return true;
}

TArray<FRichCurveEditInfoConst> UARTCurve::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> CurveEditInfos;
	for (const auto& Data : ARTCurveData)
		CurveEditInfos.Add(FRichCurveEditInfoConst(&Data.Curve, Data.Name));
	return CurveEditInfos;
}

TArray<FRichCurveEditInfo> UARTCurve::GetCurves()
{
	TArray<FRichCurveEditInfo> CurveEditInfos;
	for (auto& Data : ARTCurveData)
		CurveEditInfos.Add(FRichCurveEditInfo(&Data.Curve, Data.Name));
	return CurveEditInfos;
}

bool UARTCurve::IsValidCurve(FRichCurveEditInfo CurveInfo)
{
	for (const auto& Data : ARTCurveData)
		if (&Data.Curve == CurveInfo.CurveToEdit)
			return true;
	return false;
}

FLinearColor UARTCurve::GetCurveColor(FRichCurveEditInfo CurveInfo) const
{
	for (const auto& Data : ARTCurveData)
		if (CurveInfo.CurveToEdit == &Data.Curve)
			return Data.Color;
	return FLinearColor::White;
}


bool UARTCurve::operator==(const UARTCurve& Curve) const
{
	for (int i = 0; i < ARTCurveData.Num(); i++)
		if (!(ARTCurveData[i].Curve == Curve.ARTCurveData[i].Curve))
			return false;
	return true;
}


#if WITH_EDITOR


void UARTCurve::MakeCurveNameUnique(int CurveIdx)
{
	FARTCurveData& Curve = ARTCurveData[CurveIdx];

	// Find Name Base
	FString BaseName = Curve.Name.ToString();
	int NewNameIdx = 0;
	FString NewName = BaseName;

	int UnderscoreIdx;
	if (BaseName.FindLastChar('_', UnderscoreIdx))
	{
		FString Left = BaseName.Left(UnderscoreIdx);
		FString Right = BaseName.RightChop(UnderscoreIdx + 1);
		if (Right.IsNumeric())
		{
			BaseName = Left;
			NewNameIdx = FCString::Atoi(*Right);
		}
	}

	// Collect Names
	TSet<FName> NameList;
	for (int i = 0; i < ARTCurveData.Num(); i++)
	{
		if (i != CurveIdx)
		{
			NameList.Add(ARTCurveData[i].Name);
		}
	}

	// Find Unique Name
	while (NameList.Contains(FName(*NewName)))
	{
		NewName = FString::Printf(TEXT("%s_%d"), *BaseName, ++NewNameIdx);
	}

	Curve.Name = FName(*NewName);
}

void UARTCurve::PreEditChange(class FEditPropertyChain& PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	OldCurveCount = ARTCurveData.Num();
}

void UARTCurve::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	const FName PropName = e.GetPropertyName();
	if (PropName == GET_MEMBER_NAME_CHECKED(UARTCurve, ParentCurve))
	{
		if (ParentCurve == this)
			ParentCurve = nullptr;
	}
}

void UARTCurve::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e)
{
	const FName PropName = e.GetPropertyName();
	const FName ArrayName = e.PropertyChain.GetHead()->GetValue()->GetFName();

	if (ArrayName == GET_MEMBER_NAME_CHECKED(UARTCurve, ARTCurveData))
	{
		int CurveIdx = e.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UARTCurve, ARTCurveData));
		switch (e.ChangeType)
		{
		case EPropertyChangeType::ArrayAdd:
			if (OldCurveCount < ARTCurveData.Num())
			{
				ARTCurveData[CurveIdx].Name = NAME_CurveDefault;
				ARTCurveData[CurveIdx].Color = FLinearColor::MakeRandomColor();
				MakeCurveNameUnique(CurveIdx);
			}
			break;

		case EPropertyChangeType::Duplicate:
			// For whatever reason, duplicate adds the new item in the index before the selected
			// but we want to fix up the name on the later one, not the earlier...
			if (0 <= CurveIdx && CurveIdx + 1 < ARTCurveData.Num())
			{
				MakeCurveNameUnique(CurveIdx + 1);
			}
			break;

		case EPropertyChangeType::ValueSet:
			if (0 <= CurveIdx)
			{
				if (PropName == "Name")
				{
					MakeCurveNameUnique(CurveIdx);
				}
				if (PropName == "Color")
				{
					ARTCurveData[CurveIdx].Color.A = 1.0f;
				}
			}
			break;

		case EPropertyChangeType::ArrayClear:
			break;

		}

		OnCurveMapChanged.Broadcast(this);

		bLookupsNeedRebuild = true;

	}
}

#endif

