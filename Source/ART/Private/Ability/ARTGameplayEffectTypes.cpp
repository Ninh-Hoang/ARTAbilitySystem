// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTGameplayEffectTypes.h"

bool FARTGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	//return Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);

	uint32 RepBits = 0;

	if (Ar.IsSaving())
	{
		if (Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (EffectCauser.IsValid())
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (TargetData.IsValid())
		{
			RepBits |= 1 << 7;
		}
		if (KnockBackStrength > 0)
		{
			RepBits |= 1 << 8;
		}
		if (SourceLevel > 0)
		{
			RepBits |= 1 << 9;
		}
	}

	Ar.SerializeBits(&RepBits, 10);
	
	if (RepBits & (1 << 0))
   {
      Ar << Instigator;
   }
   if (RepBits & (1 << 1))
   {
      Ar << EffectCauser;
   }
   if (RepBits & (1 << 2))
   {
      Ar << AbilityCDO;
   }
   if (RepBits & (1 << 3))
   {
      Ar << SourceObject;
   }
   if (RepBits & (1 << 4))
   {
      SafeNetSerializeTArray_Default<31>(Ar, Actors);
   }
   if (RepBits & (1 << 5))
   {
      if (Ar.IsLoading())
      {
         if (!HitResult.IsValid())
         {
            HitResult = MakeShared<FHitResult>();
         }
      }
      HitResult->NetSerialize(Ar, Map, bOutSuccess);
   }
   if (RepBits & (1 << 6))
   {
      Ar << WorldOrigin;
      bHasWorldOrigin = true;
   }
   else
   {
      bHasWorldOrigin = false;
   }
	if (RepBits & (1 << 7))
	{
		if (Ar.IsLoading())
		{
			if (!TargetData.IsValid())
			{
				TargetData = MakeShared<FGameplayAbilityTargetDataHandle>();
			}
		}
		TargetData->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 8))
	{
		Ar << KnockBackStrength;
	}
	if (RepBits & (1 << 9))
	{
		Ar << SourceLevel;
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}
	
	bOutSuccess = true;
	return true;
}
