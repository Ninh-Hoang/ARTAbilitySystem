// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "Ability/ARTGlobalTags.h"
#include "ARTCharacter/ARTSurvivor.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "ARTCharacter/ARTCharacterBase.h"
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

	virtual ~FARTTargetFilter() override
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

	virtual ~FARTTargetFilterTeamID() override
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagContainer RequiredTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagContainer BlockedTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true, Categories = "Behaviour"), Category = Filter)
	FGameplayTagContainer BehaviourTags;
	
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

		if (!TargetActor)
		{
			bPassFilter = false;
			return bReverseFilter ^ bPassFilter;
		}
		else
		{
			if (!BehaviourTags.IsEmpty())
			{
				if(!GetTeamAttitudeTags(SelfActor, TargetActor).HasAll(BehaviourTags))
				{
					bPassFilter = false;
					return bReverseFilter ^ bPassFilter;
				}
			}

			if(!RequiredTags.IsEmpty() || !BlockedTags.IsEmpty())
			{
				UAbilitySystemComponent* ASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();;
				if(ASC)
				{
					if(!ASC->HasAllMatchingGameplayTags(RequiredTags))
					{
						bPassFilter = false;
						return bReverseFilter ^ bPassFilter;
					}
					if(ASC->HasAnyMatchingGameplayTags(BlockedTags))
					{
						bPassFilter = false;
						return bReverseFilter ^ bPassFilter;
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
				return bReverseFilter ^ bPassFilter;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_NoSelf:
			if (TargetActor == SelfActor)
			{
				bPassFilter = false;
				return bReverseFilter ^ bPassFilter;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_Any:
		default:
			break;
		}

		return bReverseFilter ^ bPassFilter;
	}

	FGameplayTagContainer GetTeamAttitudeTags(const AActor* Actor, const AActor* Other) const
	{
		FGameplayTagContainer RelationshipTags;
	
		if(!Actor || !Other) return RelationshipTags;
	
		if (Actor == Other)
		{
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Friendly);
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Self);
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Visible);
			return RelationshipTags;
		}

		const IGenericTeamAgentInterface* SourceCharacter = Cast<IGenericTeamAgentInterface>(Actor);
	
		const ETeamAttitude::Type TeamAttitude = SourceCharacter->GetTeamAttitudeTowards(*Other);

		switch (TeamAttitude)
		{
		case ETeamAttitude::Friendly:
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Friendly);
			break;
		case ETeamAttitude::Neutral:
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Neutral);
			break;
		case ETeamAttitude::Hostile:
			RelationshipTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Hostile);
			break;
		default:
			break;
		}
		return RelationshipTags;
	}
};
