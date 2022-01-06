// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "ARTGameplayEffectTypes.generated.h"

/**
 * Data structure that stores an instigator and related data, such as positions and targets
 * Games can subclass this structure and add game-specific information
 * It is passed throughout effect execution so it is a great place to track transient information about an execution
 */
USTRUCT()
struct ART_API FARTGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_USTRUCT_BODY()

public:

	//targetdata
	virtual const FGameplayAbilityTargetDataHandle* GetTargetData() const
	{
		return const_cast<FARTGameplayEffectContext*>(this)->GetTargetData();
	}

	virtual FGameplayAbilityTargetDataHandle* GetTargetData()
	{
		return TargetData.Get();
	}

	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& InTargetData, bool bReset)
	{
		if(!TargetData.IsValid())
		{
			TargetData = MakeShared<FGameplayAbilityTargetDataHandle>();
		}
		if (bReset)
		{
			TargetData.Reset();
		}

		//check(!TargetData.IsValid());
		TargetData->Append(InTargetData);
	}

	//knock back strenght
	float GetKnockBackStrength() const { return KnockBackStrength; }
	void SetKnockBackStrength(float InKnockBackStrength) { KnockBackStrength = InKnockBackStrength; }
	

	//source level
	float GetSourceLevel() const { return SourceLevel; }
	void SetSourceLevel(float InLevel) { SourceLevel = InLevel; }

	/**
	* Functions that subclasses of FGameplayEffectContext need to override
	*/

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual FARTGameplayEffectContext* Duplicate() const override
	{
		FARTGameplayEffectContext* NewContext = new FARTGameplayEffectContext();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}

		if (GetTargetData())
		{
			// Does a deep copy of the target data
			NewContext->AddTargetData(*GetTargetData(), true);
		}
		// Shallow copy of TargetData, is this okay?
		//NewContext->TargetData.Append(TargetData);
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	TSharedPtr<FGameplayAbilityTargetDataHandle> TargetData;

	UPROPERTY()
	float KnockBackStrength;
	
	UPROPERTY()
	float SourceLevel;
};

template <>
struct TStructOpsTypeTraits<FARTGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FARTGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
