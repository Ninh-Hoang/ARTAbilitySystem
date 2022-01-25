// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AI/Navigation/InfNavMesh.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

const AInfNavMesh* UInfMapFunctionLibrary::GetInfNavMesh(const UObject* WorldContext)
{
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(WorldContext->GetWorld());
	for (const auto& NavDataConfig : NavSys->GetSupportedAgents())
	{
		if (NavDataConfig.GetNavDataClass<AInfNavMesh>().Get())
		{
			return Cast<AInfNavMesh>(NavSys->GetNavDataForProps(NavDataConfig));
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
