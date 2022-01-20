// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTSelectComponent.h"


// Sets default values for this component's properties
UARTSelectComponent::UARTSelectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bSelected = false;
	// ...
}


// Called when the game starts
void UARTSelectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UARTSelectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UARTSelectComponent::SetSelected(bool Selected)
{
	if(bSelected != Selected)
	{
		bSelected = Selected;
		if(bSelected)
		{
			OnSelected.Broadcast();
		}
		else
		{
			OnDeselected.Broadcast();
		}
		return true;
	}
	return false;
}

bool UARTSelectComponent::InternalSetSelected(bool Selected)
{
	if(bSelected != Selected)
	{
		bSelected = Selected;
		if(bSelected)
		{
			OnSelected.Broadcast();
		}
		else
		{
			OnDeselected.Broadcast();
		}
		return true;
	}
	return false;
}

bool UARTSelectComponent::IsSelected()
{
	return bSelected;
}
