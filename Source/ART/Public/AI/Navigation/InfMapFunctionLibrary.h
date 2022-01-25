// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InfMapFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UInfMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static const class AInfNavMesh* GetInfNavMesh(const UObject* WorldContext);
	static void DestroyAllButFirstSpawnActor(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);
	static FColor ConvertInfluenceValueToColor(float Value);
};
