// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <ARTCharacter/ARTSurvivor.h>
#include <ARTCharacter/AI/ARTCharacterAI.h>
#include <ARTCharacter/ARTCharacterBase.h>

#include "ARTTargetFilter.generated.h"

/**
 * 
 */

/** Set rather it is possible to target self */
UENUM(BlueprintType)
namespace EARTTargetSelectionFilter
{
	enum Type
	{
		Everything UMETA(DisplayName = "Target Everything"),
		Damageable UMETA(DisplayName = "Targe Damageable"),
		Pawns UMETA(DisplayName = "Target Pawns"),
		Players UMETA(DisplayName = "Target Players"),
		AIPawns UMETA(DisplayName = "Target AI Pawns"),
		Instigator UMETA(DisplayName = "Target Self")
	};
}

//Filter with target type
USTRUCT(BlueprintType)
struct ART_API FARTTargetFilter : public FGameplayTargetDataFilter
{
	GENERATED_USTRUCT_BODY()

	virtual ~FARTTargetFilter()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	TEnumAsByte<EARTTargetSelectionFilter::Type> ActorTypeFilter;

	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override
	{
		return TargetPassesFilter(ActorToBeFiltered);
	}

	/** Returns true if the actor passes the filter and will be targeted */
	bool TargetPassesFilter(const AActor* TargetActor) const
	{
		if (!SelfActor || !TargetActor)
		{
			return false;
		}

		bool bPassFilter = true;

		if (RequiredActorClass != nullptr)
		{
			bPassFilter = RequiredActorClass == TargetActor->GetClass();
		}

		switch (ActorTypeFilter)
		{
		case EARTTargetSelectionFilter::Everything:
			break;
		case EARTTargetSelectionFilter::Damageable:
			bPassFilter = TargetActor->CanBeDamaged();
			break;
		case EARTTargetSelectionFilter::Players:
			bPassFilter = bPassFilter && TargetActor->IsA<AARTSurvivor>();
			break;
		case EARTTargetSelectionFilter::AIPawns:
			bPassFilter = bPassFilter && TargetActor->IsA<AARTCharacterAI>();
			break;
		case EARTTargetSelectionFilter::Instigator:
			bPassFilter = bPassFilter && TargetActor == SelfActor;
			break;
		default:
			break;
		}

		switch (SelfFilter.GetValue())
		{
		case ETargetDataFilterSelf::Type::TDFS_NoOthers:
			if (TargetActor != SelfActor)
			{
				bPassFilter = false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_NoSelf:
			if (TargetActor == SelfActor)
			{
				bPassFilter = false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_Any:
		default:
			break;
		}

		return bReverseFilter ^ bPassFilter;
	}
};


//filter with team attitude
USTRUCT(BlueprintType)
struct ART_API FARTTargetFilterTeamID : public FGameplayTargetDataFilter
{
	GENERATED_USTRUCT_BODY()

	virtual ~FARTTargetFilterTeamID()
	{
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	TEnumAsByte<ETeamAttitude::Type> TeamAttitude;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	bool IgnoreTeamAttitude;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagContainer RequiredTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagContainer BlockedTags;
	
	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override
	{
		return TargetPassesFilter(ActorToBeFiltered);
	}

	/** Returns true if the actor passes the filter and will be targeted */
	bool TargetPassesFilter(const AActor* TargetActor) const
	{
		bool bPassFilter = true;
	
		if (RequiredActorClass != nullptr)
		{
			bPassFilter = RequiredActorClass == TargetActor->GetClass();
		}

		if (SelfActor == nullptr || !TargetActor)
		{
			bPassFilter = false;
		}
		else
		{
			AARTCharacterBase* SourceCharacter = Cast<AARTCharacterBase>(SelfActor);

			if (!IgnoreTeamAttitude && TeamAttitude != (SourceCharacter->GetTeamAttitudeTowards(*TargetActor)))
			{
				bPassFilter = false;
			}

			if(RequiredTags.IsValid() || BlockedTags.IsValid())
			{
				UAbilitySystemComponent* ASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();;
				if(ASC)
				{
					if(!ASC->HasAllMatchingGameplayTags(RequiredTags))
					{
						bPassFilter = false;
					}
					if(ASC->HasAnyMatchingGameplayTags(BlockedTags))
					{
						bPassFilter = false;
					}
				}
			}
		}

		switch (SelfFilter.GetValue())
		{
		case ETargetDataFilterSelf::Type::TDFS_NoOthers:
			if (TargetActor != SelfActor)
			{
				bPassFilter = false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_NoSelf:
			if (TargetActor == SelfActor)
			{
				bPassFilter = false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_Any:
		default:
			break;
		}

		return bReverseFilter ^ bPassFilter;
	}
};
