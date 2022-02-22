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
struct ART_API FARTTargetDataFilter_TargetType : public FGameplayTargetDataFilter
{
	GENERATED_USTRUCT_BODY()

	virtual ~FARTTargetDataFilter_TargetType() override
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

		if (RequiredActorClass != nullptr)
		{
			if(RequiredActorClass != TargetActor->GetClass())
			{
				return bReverseFilter ^ false;
			}
		}
		
		switch (ActorTypeFilter)
		{
		case EARTTargetSelectionFilter::Everything:
			break;
		case EARTTargetSelectionFilter::Damageable:
			if(!TargetActor->CanBeDamaged()) return bReverseFilter ^ false;
			break;
		case EARTTargetSelectionFilter::Players:
			if(!TargetActor->IsA<AARTSurvivor>()) return bReverseFilter ^ false;
			break;
		case EARTTargetSelectionFilter::AIPawns:
			if(!TargetActor->IsA<AARTCharacterAI>()) return bReverseFilter ^ false;
			break;
		case EARTTargetSelectionFilter::Instigator:
			if(TargetActor != SelfActor) return bReverseFilter ^ false;
			break;
		default:
			break;
		}
		
		switch (SelfFilter.GetValue())
		{
		case ETargetDataFilterSelf::Type::TDFS_NoOthers:
			if (TargetActor != SelfActor)
			{
				return bReverseFilter ^ false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_NoSelf:
			if (TargetActor == SelfActor)
			{
				return bReverseFilter ^ false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_Any:
		default:
			break;
		}
		return bReverseFilter ^ true;
	}
};


//filter with team attitude
USTRUCT(BlueprintType)
struct ART_API FARTTargetDataFilter_TeamAttitude : public FGameplayTargetDataFilter
{
	GENERATED_USTRUCT_BODY()

	virtual ~FARTTargetDataFilter_TeamAttitude() override
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
		if (!SelfActor || !TargetActor)
		{
			return false;
		}
		if (RequiredActorClass != nullptr)
		{
			if(RequiredActorClass != TargetActor->GetClass())
			{
				return bReverseFilter ^ false;
			}
		}
		else
		{
			if (!BehaviourTags.IsEmpty())
			{
				if(!GetTeamAttitudeTags(SelfActor, TargetActor).HasAll(BehaviourTags))
				{
					return bReverseFilter ^ false;
				}
			}

			if(!RequiredTags.IsEmpty() || !BlockedTags.IsEmpty())
			{
				UAbilitySystemComponent* ASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();;
				if(ASC)
				{
					if(!ASC->HasAllMatchingGameplayTags(RequiredTags))
					{
						return bReverseFilter ^ false;
					}
					if(ASC->HasAnyMatchingGameplayTags(BlockedTags))
					{
						return bReverseFilter ^ false;
					}
				}
			}
		}

		switch (SelfFilter.GetValue())
		{
		case ETargetDataFilterSelf::Type::TDFS_NoOthers:
			if (TargetActor != SelfActor)
			{
				return bReverseFilter ^ false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_NoSelf:
			if (TargetActor == SelfActor)
			{
				return bReverseFilter ^ false;
			}
			break;
		case ETargetDataFilterSelf::Type::TDFS_Any:
		default:
			break;
		}

		return bReverseFilter ^ true;
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
