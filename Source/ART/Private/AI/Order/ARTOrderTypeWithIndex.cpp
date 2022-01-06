// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTOrderTypeWithIndex.h"
#include "GameFramework/Actor.h"
#include "AI/Order/ARTOrder.h"

//Order type with index
FARTOrderTypeWithIndex::FARTOrderTypeWithIndex()
{
	OrderType = nullptr;
	Index = -1;
	OrderTags = FGameplayTagContainer::EmptyContainer;
}

FARTOrderTypeWithIndex::FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex)
	: OrderType(InOrderType)
	  , Index(InIndex)
	  , OrderTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderTypeWithIndex::FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, FGameplayTagContainer InOrderTags)
	: OrderType(InOrderType)
	, Index(-1)
	, OrderTags(InOrderTags)
{
}

FARTOrderTypeWithIndex::FARTOrderTypeWithIndex(TSoftClassPtr<UARTOrder> InOrderType, int32 InIndex,
                                               FGameplayTagContainer InOrderTags)
	: OrderType(InOrderType)
	, Index(InIndex)
	, OrderTags(InOrderTags)
{
}

FString FARTOrderTypeWithIndex::ToString() const
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
	if (!OrderTags.IsEmpty())
	{
		if(Index >= 0)
		{
			s += TEXT(", OrderTags: ");
		}
		else
		{
			s += TEXT("OrderTags:  ");
		}
		s += FString::FromInt(Index);
	}

	s += TEXT(")");

	return s;
}

bool FARTOrderTypeWithIndex::operator==(const FARTOrderTypeWithIndex& Other) const
{
	return OrderType == Other.OrderType && Index == Other.Index;
}

bool FARTOrderTypeWithIndex::operator!=(const FARTOrderTypeWithIndex& Other) const
{
	return !(*this == Other);
}
