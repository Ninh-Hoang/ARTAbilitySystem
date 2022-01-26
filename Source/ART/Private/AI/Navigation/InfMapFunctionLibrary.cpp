// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMapFunctionLibrary.h"

#include "AISystem.h"
#include "AI/Navigation/InfNavMesh.h"
#include "NavigationSystem.h"
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

		Result[i]->MarkPendingKill();
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

FMapOperationResult UInfMapFunctionLibrary::AddTargetMap(const FMapOperationResult& MapA,
	const TMap<FIntVector, float>& MapB, float Weight)
{
	FMapOperationResult Result = MapA;
	Result.HighestPair = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);

	for (auto& Pair : Result.ResultMap)
	{
		if (const float* Value = MapB.Find(Pair.Key))
		{
			Pair.Value += (*Value) * Weight;
		}
		if (Result.HighestPair.Value < Pair.Value)
		{
			Result.HighestPair = Pair;
		}
	}

	return Result;
}

FMapOperationResult UInfMapFunctionLibrary::MultTargetMap(const FMapOperationResult& MapA,
	const TMap<FIntVector, float>& MapB, float Weight)
{
	FMapOperationResult Result = MapA;
	Result.HighestPair = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);

	for (auto& Pair : Result.ResultMap)
	{
		if (const float* Value = MapB.Find(Pair.Key))
			Pair.Value *= (*Value) * Weight;
		else
			Pair.Value = 0.f;

		if (Result.HighestPair.Value < Pair.Value)
		{
			Result.HighestPair = Pair;
		}
	}

	return Result;
}

FMapOperationResult UInfMapFunctionLibrary::InvertTargetMap(const FMapOperationResult& TargetMap)
{
	FMapOperationResult Result = TargetMap;
	Result.HighestPair = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);

	for (auto& Pair : Result.ResultMap)
	{
		Pair.Value = TargetMap.HighestPair.Value - Pair.Value;
		if (Result.HighestPair.Value < Pair.Value)
			Result.HighestPair = Pair;
	}

	return Result;
}

FMapOperationResult UInfMapFunctionLibrary::NormalizeTargetMap(const FMapOperationResult& TargetMap)
{
	FMapOperationResult Result = TargetMap;
	Result.HighestPair = TPair<FIntVector, float>(FIntVector(), -FLT_MAX);

	for (auto& Pair : Result.ResultMap)
	{
		Pair.Value /= TargetMap.HighestPair.Value;
		if (Result.HighestPair.Value < Pair.Value)
			Result.HighestPair = Pair;
	}

	return Result;
}

FIntVector UInfMapFunctionLibrary::GetHighestLocation(const FMapOperationResult& TargetMap)
{
	return TargetMap.HighestPair.Key;
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
