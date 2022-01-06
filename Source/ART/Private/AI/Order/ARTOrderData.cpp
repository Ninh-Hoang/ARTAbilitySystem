// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderData.h"
#include "GameFramework/Actor.h"

FARTOrderData::FARTOrderData()
{
    OrderType = nullptr;
    bUseLocation = false;
    Location = FVector2D::ZeroVector;
    Target = nullptr;
    Index = -1;
    OrderTags = FGameplayTagContainer::EmptyContainer;
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(nullptr)
    , Index(-1)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, AActor* InTarget)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(InTarget)
    , Index(-1)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(nullptr)
    , Index(-1)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, AActor* InTarget, FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(InTarget)
    , Index(-1)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(nullptr)
    , Index(InIndex)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(nullptr)
    , Index(InIndex)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, AActor* InTarget)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(InTarget)
    , Index(InIndex)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, AActor* InTarget,
                             FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(InTarget)
    , Index(InIndex)
    , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(nullptr)
    , Index(InIndex)
    , OrderTags(InOrderTags)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags, AActor* InTarget)
    : OrderType(InOrderType)
    , bUseLocation(false)
    , Location(FVector2D::ZeroVector)
    , Target(InTarget)
    , Index(InIndex)
    , OrderTags(InOrderTags)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags,
    FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(nullptr)
    , Index(InIndex)
    , OrderTags(InOrderTags)
{
}

FARTOrderData::FARTOrderData(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex, FGameplayTagContainer InOrderTags, AActor* InTarget,
    FVector2D InLocation)
    : OrderType(InOrderType)
    , bUseLocation(true)
    , Location(InLocation)
    , Target(InTarget)
    , Index(InIndex)
    , OrderTags(InOrderTags)
{
}

FString FARTOrderData::ToString() const
{
    FString s;

    if (OrderType != nullptr)
    {
        if (!OrderType.IsValid())
        {
            OrderType.LoadSynchronous();
        }

        s += OrderType->GetName();
    }
    else
    {
        s += TEXT("Unknown Order");
    }

    s += TEXT(" (");

    if (Index >= 0)
    {
        s += TEXT("Index: ");
        s += FString::FromInt(Index);
    }

    if (Target != nullptr)
    {
        if (Index >= 0)
        {
            s += TEXT(", Target: ");
        }

        else
        {
            s += TEXT("Target: ");
        }

        s += Target->GetName();
    }

    if (bUseLocation)
    {
        if (Target != nullptr || Index >= 0)
        {
            s += TEXT(", Location: ");
        }

        else
        {
            s += TEXT("Location: ");
        }

        s += Location.ToString();
    }
    
    if (!OrderTags.IsEmpty())
    {
        if (Target != nullptr || Index >= 0 || bUseLocation)
        {
            s += TEXT(", OrderTags: ");
        }

        else
        {
            s += TEXT("OrderTags: ");
        }

        s += OrderTags.ToString();
    }

    s += TEXT(")");

    return s;
}



bool FARTOrderData::operator==(const FARTOrderData& Other) const
{
    bool bEqual =
        OrderType == Other.OrderType && bUseLocation == bUseLocation && Target == Other.Target && Index == Other.Index && OrderTags == Other.OrderTags;

    if (bUseLocation)
    {
        bEqual = bEqual && Location.Equals(Other.Location);
    }

    return bEqual;
}

bool FARTOrderData::operator!=(const FARTOrderData& Other) const
{
    return !(*this == Other);
}