// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/TargetActor/GATA_SphereTrace.h"
#include "WorldCollision.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AGATA_SphereTrace::AGATA_SphereTrace()
{
	TraceSphereRadius = 300;
}

void AGATA_SphereTrace::Configure(const FGameplayAbilityTargetingLocationInfo& InStartLocation,
                                  FGameplayTag InAimingTag, FGameplayTag InAimingRemovalTag,
                                  FCollisionProfileName InTraceProfile, FGameplayTargetDataFilterHandle InFilter,
                                  TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
                                  FWorldReticleParameters InReticleParams,
                                  bool bInIgnoreBlockingHits /*= false*/,
                                  bool bInShouldProduceTargetDataOnServer /*= false*/,
                                  bool bInUsePersistentHitResults /*= false*/,
                                  bool bInIgnoreSourceActor /*= false*/, bool bInDebug /*= false*/,
                                  bool bInTraceAffectsAimPitch /*= true*/,
                                  bool bInTraceWithPawnOrientation /*=true*/,
                                  bool bInTraceFromPlayerViewPoint /*= false*/, bool bInUseAimingSpreadMod /*= false*/,
                                  float InMaxRange /*= 999999.0f*/, float InTraceSphereRadius /*= 100.0f*/,
                                  float InBaseSpread /*= 0.0f*/, float InAimingSpreadMod /*= 0.0f*/,
                                  float InTargetingSpreadIncrement /*= 0.0f*/, float InTargetingSpreadMax /*= 0.0f*/,
                                  int32 InMaxHitResultsPerTrace /*= 1*/, int32 InNumberOfTraces /*= 1 */)
{
	StartLocation = InStartLocation;
	AimingTag = InAimingTag;
	AimingRemovalTag = InAimingRemovalTag;
	TraceProfile = InTraceProfile;
	Filter = InFilter;
	ReticleClass = InReticleClass;
	ReticleParams = InReticleParams;
	bIgnoreBlockingHits = bInIgnoreBlockingHits;
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
	bUsePersistentHitResults = bInUsePersistentHitResults;
	bIgnoreSourceActor = bInIgnoreSourceActor;
	bDebug = bInDebug;
	bTraceAffectsAimPitch = bInTraceAffectsAimPitch;
	bTraceWithPawnOrientation = bInTraceWithPawnOrientation;
	bTraceFromPlayerViewPoint = bInTraceFromPlayerViewPoint;
	bUseAimingSpreadMod = bInUseAimingSpreadMod;
	MaxRange = InMaxRange;
	TraceSphereRadius = InTraceSphereRadius;
	BaseSpread = InBaseSpread;
	AimingSpreadMod = InAimingSpreadMod;
	TargetingSpreadIncrement = InTargetingSpreadIncrement;
	TargetingSpreadMax = InTargetingSpreadMax;
	MaxHitResultsPerTrace = InMaxHitResultsPerTrace;
	NumberOfTraces = InNumberOfTraces;

	if (bUsePersistentHitResults)
	{
		NumberOfTraces = 1;
	}
}

void AGATA_SphereTrace::SphereTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
                                              const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                                              const FVector& End, float Radius, FName ProfileName,
                                              const FCollisionQueryParams Params)
{
	check(World);

	TArray<FHitResult> HitResults;
	World->SweepMultiByProfile(HitResults, Start, End, FQuat::Identity, ProfileName,
	                           FCollisionShape::MakeSphere(Radius), Params);

	TArray<FHitResult> FilteredHitResults;

	// Start param could be player ViewPoint. We want HitResult to always display the StartLocation.
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.GetActor() || FilterHandle.FilterPassesForActor(Hit.GetActor()))
		{
			Hit.TraceStart = TraceStart;
			Hit.TraceEnd = End;

			FilteredHitResults.Add(Hit);
		}
	}

	OutHitResults = FilteredHitResults;
}

void AGATA_SphereTrace::DoTrace(TArray<FHitResult>& HitResults, const UWorld* World,
                                const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                                const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	SphereTraceWithFilter(HitResults, World, FilterHandle, Start, End, TraceSphereRadius, ProfileName, Params);
}

void AGATA_SphereTrace::ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType,
                                       float Duration)
{
#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		FVector ViewStart = StartLocation.GetTargetingTransform().GetLocation();
		FRotator ViewRot;
		if (MasterPC && bTraceFromPlayerViewPoint)
		{
			MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);
		}

		FVector TraceEnd = HitResults[0].TraceEnd;
		if (NumberOfTraces > 1 || bUsePersistentHitResults)
		{
			TraceEnd = CurrentTraceEnd;
		}

		DrawDebugSphereTraceMulti(GetWorld(), ViewStart, TraceEnd, TraceSphereRadius, DrawDebugType, true, HitResults,
		                          FLinearColor::Green, FLinearColor::Red, Duration);
	}
#endif
}

#if ENABLE_DRAW_DEBUG
// Copied from KismetTraceUtils.cpp
void AGATA_SphereTrace::DrawDebugSweptSphere(const UWorld* InWorld, FVector const& Start, FVector const& End,
                                             float Radius, FColor const& Color, bool bPersistentLines, float LifeTime,
                                             uint8 DepthPriority)
{
	FVector const TraceVec = End - Start;
	float const Dist = TraceVec.Size();

	FVector const Center = Start + TraceVec * 0.5f;
	float const HalfHeight = (Dist * 0.5f) + Radius;

	FQuat const CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	DrawDebugCapsule(InWorld, Center, HalfHeight, Radius, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
}

void AGATA_SphereTrace::DrawDebugSphereTraceMulti(const UWorld* World, const FVector& Start, const FVector& End,
                                                  float Radius, EDrawDebugTrace::Type DrawDebugType, bool bHit,
                                                  const TArray<FHitResult>& OutHits, FLinearColor TraceColor,
                                                  FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHits.Last().bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			FVector const BlockingHitPoint = OutHits.Last().Location;
			DrawDebugSweptSphere(World, Start, BlockingHitPoint, Radius, TraceColor.ToFColor(true), bPersistent,
			                     LifeTime);
			DrawDebugSweptSphere(World, BlockingHitPoint, End, Radius, TraceHitColor.ToFColor(true), bPersistent,
			                     LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugSweptSphere(World, Start, End, Radius, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}

		// draw hits
		for (int32 HitIdx = 0; HitIdx < OutHits.Num(); ++HitIdx)
		{
			FHitResult const& Hit = OutHits[HitIdx];
			DrawDebugPoint(World, Hit.ImpactPoint, 16.0f,
			               (Hit.bBlockingHit ? TraceColor.ToFColor(true) : TraceHitColor.ToFColor(true)), bPersistent,
			               LifeTime);
		}
	}
}
#endif // ENABLE_DRAW_DEBUG
