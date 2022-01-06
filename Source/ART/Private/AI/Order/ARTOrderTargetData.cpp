﻿#include "AI/Order/ARTOrderTargetData.h"
#include "GameFramework/Actor.h"

//target data
FARTOrderTargetData::FARTOrderTargetData()
	: Actor(nullptr)
	, Location(FVector2D::ZeroVector)
	, TargetTags(FGameplayTagContainer::EmptyContainer)
{
}

FARTOrderTargetData::FARTOrderTargetData(AActor* InActor, const FVector2D InLocation,
										const FGameplayTagContainer& InTargetTags)
	: Actor(InActor)
	, Location(InLocation)
	, TargetTags(InTargetTags)
{
}

FString FARTOrderTargetData::ToString() const
{
	FString s;
	s += TEXT("(");
	s += TEXT("Actor: ");
	if (Actor != nullptr)
	{
		s += Actor->GetName();
	}

	else
	{
		s += FName().ToString();
	}

	s += TEXT(", Location: ");
	s += Location.ToString();

	s += TEXT(", Tags:( ");
	s += TargetTags.ToString();
	s += TEXT(")");
	s += TEXT(")");

	return s;
}