// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AISystem.h"
#include "AI/Navigation/InfNavMesh.h"
#include "NavigationSystem.h"
#include "AI/Navigation/InfPropagator.h"
#include "Kismet/GameplayStatics.h"

AInfNavMesh* UInfMapFunctionLibrary::GetInfNavMesh(const UObject* WorldContext)
{
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(WorldContext->GetWorld());
	for (const auto& NavDataConfig : NavSys->GetSupportedAgents())
	{
		if (NavDataConfig.GetNavDataClass<AInfNavMesh>().Get())
		{
			ANavigationData* NavData = const_cast<ANavigationData*>(NavSys->GetNavDataForProps(NavDataConfig));
			return Cast<AInfNavMesh>(NavData);
		}
	}

	return nullptr;
}

void UInfMapFunctionLibrary::DestroyAllButFirstSpawnActor(const UObject* WorldContextObject,
                                                          TSubclassOf<AActor> ActorClass)
{TArray<AActor*> Result;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ActorClass, Result);
	if (Result.Num() <= 1)
		return;

	for (int i = 1; i < Result.Num(); ++i)
	{

		Result[i]->MarkAsGarbage();
	}

	UE_LOG(LogTemp, Warning, TEXT("%s : There is an actor marked Pending Kill. Please reopen the level."), *ActorClass.Get()->GetFName().ToString());
}

FColor UInfMapFunctionLibrary::ConvertInfluenceValueToColor(float Value)
{
	FColor Color = FColor::Black;
	if (Value > 0.f)
		Color = FLinearColor::LerpUsingHSV(FColor::Cyan, FColor::Blue, Value).ToFColor(false);
	else if (Value < 0.f)
		Color = FLinearColor::LerpUsingHSV(FColor::Yellow, FColor::Red, FMath::Abs(Value)).ToFColor(false);
	return Color;
}

FMapOperationResult UInfMapFunctionLibrary::InitializeWorkingMap(const TMap<FIntVector, float>& NewMap)
{
	FMapOperationResult Result;
	Result.ResultMap = NewMap;
	return Result;
}

FMapOperationResult& UInfMapFunctionLibrary::AddTargetMap(const FMapOperationResult& MapA,
	const TMap<FIntVector, float>& MapB, float Weight, bool Inclusive)
{
	//evil const_cast to save from memory, probably fine as this map not gonna be used by multiple processes
	FMapOperationResult& Result = const_cast<FMapOperationResult&>(MapA);
	
	TPair<FIntVector, float>& HighestPairRef = Result.HighestPair;
	TPair<FIntVector, float>& LowestPairRef = Result.LowestPair;
	
	HighestPairRef  = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);
	LowestPairRef  = TPair<FIntVector, float>(FIntVector(), FLT_MAX);

	//TODO: What the fuck is this Yan dev nested if
	for (auto& Pair : MapB)
	{
		bool PairAdded = false;
		
		if (Result.ResultMap.Find(Pair.Key))
		{
			Result.ResultMap[Pair.Key] += Pair.Value * Weight;
			PairAdded = true;
		}
		else if(Inclusive)
		{
			Result.ResultMap.Add(Pair.Key, Pair.Value * Weight);
			PairAdded = true;
		}

		if(PairAdded)
		{
			if (HighestPairRef.Value < Pair.Value * Weight)
			{
				HighestPairRef.Key = Pair.Key;
				HighestPairRef.Value = Pair.Value * Weight;
			}
			if (Result.LowestPair.Value > Pair.Value * Weight)
			{
				LowestPairRef.Key = Pair.Key;
				LowestPairRef.Value = Pair.Value * Weight;
			}
		}
	}

	return Result;
}

FMapOperationResult& UInfMapFunctionLibrary::MultTargetMap(const FMapOperationResult& MapA,
	const TMap<FIntVector, float>& MapB, float Weight, bool Inclusive)
{
	//evil const_cast to save from memory, probably fine as this map not gonna be used by multiple processes
	FMapOperationResult& Result = const_cast<FMapOperationResult&>(MapA);

	TPair<FIntVector, float>& HighestPairRef = Result.HighestPair;
	TPair<FIntVector, float>& LowestPairRef = Result.LowestPair;
	
	HighestPairRef  = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);
	LowestPairRef  = TPair<FIntVector, float>(FIntVector(), FLT_MAX);
	
	for (auto& Pair : Result.ResultMap)
	{
		bool PairAdded = false;
		
		if (const float* Value = MapB.Find(Pair.Key)) Pair.Value *= (*Value) * Weight;
		else Pair.Value = 0.f;
		
		if (HighestPairRef.Value < Pair.Value * Weight)
		{
			HighestPairRef = Pair;
		}
		if (LowestPairRef.Value > Pair.Value * Weight)
		{
			LowestPairRef = Pair;
		}
	}

	return Result;
}

FMapOperationResult& UInfMapFunctionLibrary::InvertTargetMap(const FMapOperationResult& TargetMap)
{
	FMapOperationResult& Result = const_cast<FMapOperationResult&>(TargetMap);
	
	TPair<FIntVector, float>& HighestPairRef = Result.HighestPair;
	TPair<FIntVector, float>& LowestPairRef = Result.LowestPair;

	const float Max = HighestPairRef.Value;

	HighestPairRef  = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);
	LowestPairRef  = TPair<FIntVector, float>(FIntVector(), FLT_MAX);
	
	for (auto& Pair : Result.ResultMap)
	{
		Pair.Value = Max - Pair.Value;
		
		if (HighestPairRef.Value < Pair.Value)
		{
			HighestPairRef = Pair;
		}
		if (LowestPairRef.Value > Pair.Value)
		{
			LowestPairRef = Pair;
		}
	}

	return Result;
}

FMapOperationResult& UInfMapFunctionLibrary::NormalizeTargetMap(const FMapOperationResult& TargetMap)
{
	FMapOperationResult& Result = const_cast<FMapOperationResult&>(TargetMap);
	
	TPair<FIntVector, float>& HighestPairRef = Result.HighestPair;
	TPair<FIntVector, float>& LowestPairRef = Result.LowestPair;

	const float Max = HighestPairRef.Value;
	const float Min = LowestPairRef.Value;
	const float Range = Max - Min;

	HighestPairRef  = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);
	LowestPairRef  = TPair<FIntVector, float>(FIntVector(), FLT_MAX);
	
	for (auto& Pair : Result.ResultMap)
	{
		Pair.Value = (Pair.Value - Min) / Range;
		
		if (HighestPairRef.Value < Pair.Value)
		{
			HighestPairRef = Pair;
		}
		if (LowestPairRef.Value > Pair.Value)
		{
			LowestPairRef = Pair;
		}
	}

	return Result;
}

FIntVector UInfMapFunctionLibrary::GetHighestLocation(const FMapOperationResult& TargetMap)
{
	return TargetMap.HighestPair.Key;
}
	
FIntVector UInfMapFunctionLibrary::GetLowestLocation(const FMapOperationResult& TargetMap)
{
	return TargetMap.LowestPair.Key;
}

FIntVector UInfMapFunctionLibrary::SelectNearbyHighestLocation(const FMapOperationResult& TargetMap,
                                                               const FVector& CurrentLocation)
{
	TMap<FIntVector, float> Temp = TargetMap.ResultMap;
	// Value���傫�����Ƀ\�[�g
	Temp.ValueSort([&](float A, float B) { return A > B; });

	float MinDistanceSq = FLT_MAX;
	FIntVector Result = FIntVector::NoneValue;
	for (const auto& Pair : Temp)
	{
		if (TargetMap.HighestPair.Value != Pair.Value)
			break;

		float DistSq = FVector::DistSquared(FVector(Pair.Key), CurrentLocation);
		if (DistSq < MinDistanceSq)
		{
			MinDistanceSq = DistSq;
			Result = Pair.Key;
		}
	}

	return Result;
}

FIntVector UInfMapFunctionLibrary::SelectLocationOfLeastInfluenceValue(const FMapOperationResult& TargetMap,
	float Percent)
{
	float MinScore = TargetMap.HighestPair.Value * (1.f - Percent);

	TMap<FIntVector, float> Temp = TargetMap.ResultMap;

	Temp.ValueSort([&](float A, float B) { return A > B; });
	
	int32 NumBestItems = 0;
	for (const auto& Pair : Temp)
	{
		NumBestItems++;
		if (Pair.Value < MinScore)
			break;
	}
	
	NumBestItems = FMath::Max(1, UAISystem::GetRandomStream().RandHelper(NumBestItems));

	FIntVector TargetKey = FIntVector::NoneValue;

	auto It = Temp.CreateConstIterator();
	for (int i = 0; i < NumBestItems; ++i, ++It)
	{
		TargetKey = (*It).Key;
	}

	return TargetKey;
}

FMapOperationResult UInfMapFunctionLibrary::GetInfluenceMapFromQuery(const FInfQueryData& QueryData)
{
	TMap<FIntVector, float> TempInfluenceMap;
	FMapOperationResult ResultData = InitializeWorkingMap(TempInfluenceMap);
	UInfPropagator* Propagator = QueryData.Propagator;
	if(Propagator)
	{
		TMap<FIntVector, float> TargetMap = TMap<FIntVector, float>();
		for (const auto& OpData : QueryData.OperationConstructData)
		{
			if(OpData.OperationType == EOperationType::Add || OpData.OperationType == EOperationType::Mult)
			{
				TargetMap = Propagator->GetMap(OpData.MapTag, OpData.GatherDistance, OpData.bIgnoreSelf, OpData.BehaviourTags, OpData.RequiredTag, OpData.BlockedTag);
			}
		
			switch (OpData.OperationType)
			{
			case EOperationType::Add:
				AddTargetMap(ResultData, TargetMap, OpData.Weight, OpData.Inclusive);
				break;
			case EOperationType::Mult:
				MultTargetMap(ResultData, TargetMap, OpData.Weight, OpData.Inclusive);
				break;
			case EOperationType::Invert:
				InvertTargetMap(ResultData);
				break;
			case EOperationType::Normalize:
				NormalizeTargetMap(ResultData);
				break;
			}
		}
	}
	return ResultData;
}

FInfQueryData UInfMapFunctionLibrary::MakeInfluenceQueryData(const TArray<FOperationData>& InfluenceOperation, UInfPropagator* InPropagator)
{
	FInfQueryData Result = FInfQueryData();
	Result.OperationConstructData = InfluenceOperation;
	Result.Propagator = InPropagator;
	return Result;
}
