// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/DeprojectWidget.h"

UDeprojectWidget::UDeprojectWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UDeprojectWidget::OnViewportSizeChanged);
}

void UDeprojectWidget::OnViewportSizeChanged(FViewport* ViewPort, uint32 val)
{
	OnViewPortResized();
}


