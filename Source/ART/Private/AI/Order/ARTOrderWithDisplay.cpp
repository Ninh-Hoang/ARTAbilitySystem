// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderWithDisplay.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"


UTexture2D* UARTOrderWithDisplay::GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                int32 Index) const
{
	return NormalIcon;
}

UTexture2D* UARTOrderWithDisplay::GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	return HoveredIcon;
}

UTexture2D* UARTOrderWithDisplay::GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                 int32 Index) const
{
	return PressedIcon;
}

UTexture2D* UARTOrderWithDisplay::GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                                  int32 Index) const
{
	return DisabledIcon;
}

FText UARTOrderWithDisplay::GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                    int32 Index) const
{
	return Name;
}

FText UARTOrderWithDisplay::GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
                                           int32 Index) const
{
	return Description;
}

int32 UARTOrderWithDisplay::GetOrderButtonIndex() const
{
	return OrderButtonIndex;
}

bool UARTOrderWithDisplay::HasFixedOrderButtonIndex() const
{
	return bUseFixedOrderButtonIndex;
}

FARTOrderPreviewData UARTOrderWithDisplay::GetOrderPreviewData(const AActor* OrderedActor,
                                                               const FGameplayTagContainer& OrderTags,
                                                               int32 Index) const
{
	return OrderPreviewData;
}