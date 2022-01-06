// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeprojectWidget.generated.h"

/**
 * 
 */

UCLASS()
class ART_API UDeprojectWidget : public UUserWidget
{
	GENERATED_BODY()

	UDeprojectWidget(const FObjectInitializer& ObjectInitializer);
	
public:
    void OnViewportSizeChanged(class FViewport* ViewPort, uint32 val);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnViewPortResized();
};
