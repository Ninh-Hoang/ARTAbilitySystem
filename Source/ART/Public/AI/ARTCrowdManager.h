// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/CrowdManager.h"
#include "ARTCrowdManager.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTCrowdManager : public UCrowdManager
{
	GENERATED_BODY()
	
public:
	UARTCrowdManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** pass new velocity to movement components */
	//virtual void ApplyVelocity(UCrowdFollowingComponent* AgentComponent, int32 AgentIndex) const override;
};
