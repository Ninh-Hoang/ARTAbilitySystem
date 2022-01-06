// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/TargetActor/GATA_MeleeWeaponTrace.h"
#include <Weapon/MeleeWeapon.h>
#include <ARTCharacter/ARTCharacterBase.h>

AGATA_MeleeWeaponTrace::AGATA_MeleeWeaponTrace()
{
}

void AGATA_MeleeWeaponTrace::Configure(class AMeleeWeapon* InMeleeWeaponActor,
                                       const FGameplayAbilityTargetingLocationInfo& InStartLocation,
                                       FGameplayTag InAimingTag, FGameplayTag InAimingRemovalTag,
                                       FCollisionProfileName InTraceProfile,
                                       FGameplayTargetDataFilterHandle InFilter,
                                       TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
                                       FWorldReticleParameters InReticleParams,
                                       bool bInShouldProduceTargetDataOnServer /*= false*/,
                                       bool bInDebug /*= false*/,
                                       bool bInUsePawnSocket /*= false*/,
                                       FName InMeleeSocketName,
                                       float InSocketLenght)
{
	MeleeWeaponActor = InMeleeWeaponActor;
	StartLocation = InStartLocation;
	AimingTag = InAimingTag;
	AimingRemovalTag = InAimingRemovalTag;
	TraceProfile = InTraceProfile;
	Filter = InFilter;
	ReticleClass = InReticleClass;
	ReticleParams = InReticleParams;
	bIgnoreBlockingHits = false;
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
	bUsePersistentHitResults = true;
	bDebug = bInDebug;
	bTraceAffectsAimPitch = false;
	bTraceWithPawnOrientation = false;
	bTraceFromPlayerViewPoint = false;
	bUseAimingSpreadMod = false;
	bIgnoreSourceActor = true;
	MaxRange = 3000.f;
	BaseSpread = 0.f;
	AimingSpreadMod = 0.f;
	TargetingSpreadIncrement = 0.f;
	TargetingSpreadMax = 0.f;
	MaxHitResultsPerTrace = 30;
	NumberOfTraces = 2;
	bUsePawnSocket = bInUsePawnSocket;
	MeleeSocketName = InMeleeSocketName;
	SocketLenght = InSocketLenght;
}

//do simple line trace, additional logic when trace here
void AGATA_MeleeWeaponTrace::DoTrace(TArray<FHitResult>& HitResults, const UWorld* World,
                                     const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                                     const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	LineTraceWithFilter(HitResults, World, FilterHandle, Start, End, ProfileName, Params);
}


void AGATA_MeleeWeaponTrace::StartTargeting(UGameplayAbility* Ability)
{
	UE_LOG(LogTemp, Warning, TEXT("Length"));
	Super::StartTargeting(Ability);

	CacheHitResults.Empty();

	//reset all array
	CurrVecs.Reset();
	MyTargets.Empty();

	if (MeleeWeaponActor)
	{
		//initialize vector values
		for (int i = MeleeWeaponActor->WeaponStartLength; i <= MeleeWeaponActor->WeaponTail; i += 10)
		{
			CurrVecs.Add(MeleeWeaponActor->GetActorLocation() + i * MeleeWeaponActor->GetActorUpVector());
		}
	}
	if (bUsePawnSocket)
	{
		OwnerCharacter = Cast<AARTCharacterBase>(SourceActor);
		if (OwnerCharacter)
		{
			for (int i = 0; i <= SocketLenght; i += 10)
			{
				CurrVecs.Add(
					OwnerCharacter->GetMesh()->GetSocketLocation(MeleeSocketName) + i * FRotationMatrix(
						OwnerCharacter->GetMesh()->GetSocketRotation(MeleeSocketName)).GetScaledAxis(EAxis::Z));
			}
		}
	}
}

TArray<FHitResult> AGATA_MeleeWeaponTrace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;
	if (MeleeWeaponActor)
	{
		ActorsToIgnore.Add(MeleeWeaponActor);
	}
	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGATA_LineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bIgnoreBlocks = bIgnoreBlockingHits;

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;


	// Clear any blocking hit results, invalid Actors, or actors out of range
	//TODO Check for visibility if we add AIPerceptionComponent in the future
	for (int32 i = PersistentHitResults.Num() - 1; i >= 0; i--)
	{
		FHitResult& HitResult = PersistentHitResults[i];

		if (!HitResult.GetActor() || FVector::DistSquared(TraceStart, HitResult.GetActor()->GetActorLocation()) >
			(MaxRange * MaxRange))
		{
			PersistentHitResults.RemoveAt(i);
		}
	}

	SetActorLocationAndRotation(TraceStart, SourceActor->GetActorRotation());

	TArray<FHitResult> TraceHitResults;

	PrevVecs = CurrVecs;

	float StartLenght = 0.0f;
	float EndLenght = 0.0f;

	if (MeleeWeaponActor)
	{
		StartLenght = MeleeWeaponActor->WeaponStartLength;
		EndLenght = MeleeWeaponActor->WeaponTail;
	}
	else if (bUsePawnSocket)
	{
		StartLenght = 0.0f;
		EndLenght = SocketLenght;
	}

	for (int i = StartLenght; i <= EndLenght; i += 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("Length"));
		const int32 size = (bUsePawnSocket) ? i : (i - MeleeWeaponActor->WeaponStartLength) / 10;

		if (MeleeWeaponActor)
		{
			CurrVecs[size] = MeleeWeaponActor->GetActorLocation() + i * MeleeWeaponActor->GetActorUpVector();
		}
		if (bUsePawnSocket)
		{
			CurrVecs[size] = OwnerCharacter->GetMesh()->GetSocketLocation(MeleeSocketName) + i * FRotationMatrix(
				OwnerCharacter->GetMesh()->GetSocketRotation(MeleeSocketName)).GetScaledAxis(EAxis::Z);
		}

		TArray<FHitResult> TraceHitResultsBuffer;
		DoTrace(TraceHitResultsBuffer, InSourceActor->GetWorld(), Filter, PrevVecs[size], CurrVecs[size],
		        TraceProfile.Name, Params);

		TraceHitResults.Append(TraceHitResultsBuffer);

		DrawDebugLineTraceMulti(GetWorld(), PrevVecs[size], CurrVecs[size], EDrawDebugTrace::Type::ForDuration, true,
		                        TraceHitResults, FLinearColor::Red, FLinearColor::Green, 2.0f);
	}

	for (int32 j = TraceHitResults.Num() - 1; j >= 0; j--)
	{
		FHitResult& HitResult = TraceHitResults[j];

		// Reminder: if bUsePersistentHitResults, Number of Traces = 1

		// This is looping backwards so that further objects from player are added first to the queue.
		// This results in closer actors taking precedence as the further actors will get bumped out of the TArray.
		if (HitResult.GetActor() /*&& (!HitResult.bBlockingHit || PersistentHitResults.Num() < 1)*/)
		{
			bool bActorAlreadyInPersistentHits = false;

			// Make sure PersistentHitResults doesn't have this hit actor already
			for (int32 k = 0; k < CacheHitResults.Num(); k++)
			{
				FHitResult& CacheHitResult = CacheHitResults[k];

				if (CacheHitResult.GetActor() == HitResult.GetActor())
				{
					bActorAlreadyInPersistentHits = true;
					break;
				}
			}

			if (bActorAlreadyInPersistentHits)
			{
				continue;
			}

			if (PersistentHitResults.Num() >= MaxHitResultsPerTrace)
			{
				// Treat PersistentHitResults like a queue, remove first element
				PersistentHitResults.RemoveAt(0);
			}

			PersistentHitResults.Add(HitResult);
			CacheHitResults.Add(HitResult);

			ValidData.Broadcast();
		}
	} // for TraceHitResults

	if (TraceHitResults.Num() < ReticleActors.Num())
	{
		// We have less hit results than ReticleActors, hide the extra ones
		for (int32 j = TraceHitResults.Num(); j < ReticleActors.Num(); j++)
		{
			if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[j].Get())
			{
				LocalReticleActor->SetIsTargetAnActor(false);
				LocalReticleActor->SetActorHiddenInGame(true);
			}
		}
	}


	if (TraceHitResults.Num() < 1)
	{
		// If there were no hits, add a default HitResult at the end of the trace
		FHitResult HitResult;
		// Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
		HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
		HitResult.TraceEnd = TraceEnd;
		HitResult.Location = TraceEnd;
		HitResult.ImpactPoint = TraceEnd;
		TraceHitResults.Add(HitResult);

		if (PersistentHitResults.Num() < 1)
		{
			PersistentHitResults.Add(HitResult);
		}
	}

	if (ReticleActors.Num() <= 0)
	{
		return PersistentHitResults;
	}

	// Handle ReticleActors
	for (int32 PersistentHitResultIndex = 0; PersistentHitResultIndex < PersistentHitResults.Num();
	     PersistentHitResultIndex++)
	{
		FHitResult& HitResult = PersistentHitResults[PersistentHitResultIndex];

		// Update TraceStart because old persistent HitResults will have their original TraceStart and the player could have moved since then
		HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();

		if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
		{
			const bool bHitActor = HitResult.GetActor() != nullptr;

			if (bHitActor && !HitResult.bBlockingHit)
			{
				LocalReticleActor->SetActorHiddenInGame(false);

				const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor)
					                                ? HitResult.GetActor()->GetActorLocation()
					                                : HitResult.Location;

				LocalReticleActor->SetActorLocation(ReticleLocation);
				LocalReticleActor->SetIsTargetAnActor(bHitActor);
			}
			else
			{
				LocalReticleActor->SetActorHiddenInGame(true);
			}
		}
	}

	if (PersistentHitResults.Num() < ReticleActors.Num())
	{
		// We have less hit results than ReticleActors, hide the extra ones
		for (int32 PersistentHitResultIndex = PersistentHitResults.Num(); PersistentHitResultIndex < ReticleActors.Num()
		     ; PersistentHitResultIndex++)
		{
			if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
			{
				LocalReticleActor->SetIsTargetAnActor(false);
				LocalReticleActor->SetActorHiddenInGame(true);
			}
		}
	}

	return PersistentHitResults;
}

void AGATA_MeleeWeaponTrace::ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType,
                                            float Duration /*= 2.0f*/)
{
#if ENABLE_DRAW_DEBUG
	//not using this, overrided
#endif
}

void AGATA_MeleeWeaponTrace::DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End,
                                                     EDrawDebugTrace::Type DrawDebugType, bool bHit,
                                                     const TArray<FHitResult>& OutHits, FLinearColor TraceColor,
                                                     FLinearColor TraceHitColor, float DrawTime)
{
#if ENABLE_DRAW_DEBUG
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		// @fixme, draw line with thickness = 2.f?
		/*if (bHit && OutHits.Last().bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			FVector const BlockingHitPoint = OutHits.Last().ImpactPoint;
			::DrawDebugLine(World, Start, BlockingHitPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			::DrawDebugLine(World, BlockingHitPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{*/
		// no hit means all red
		DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		//}

		// draw hits
		for (int32 HitIdx = 0; HitIdx < OutHits.Num(); ++HitIdx)
		{
			FHitResult const& Hit = OutHits[HitIdx];
			DrawDebugPoint(World, Hit.ImpactPoint, 16.0f,
			               (!Hit.bBlockingHit ? TraceColor.ToFColor(true) : TraceHitColor.ToFColor(true)), bPersistent,
			               LifeTime);
		}
	}
#endif
}
