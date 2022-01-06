// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/TargetActor/GATA_LineTrace.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AGATA_LineTrace::AGATA_LineTrace()
{
}

void AGATA_LineTrace::Configure(
	AActor* InReferenceActor,
	const FGameplayAbilityTargetingLocationInfo& InStartLocation,
	FGameplayTag InAimingTag,
	FGameplayTag InAimingRemovalTag,
	FCollisionProfileName InTraceProfile,
	FGameplayTargetDataFilterHandle InFilter,
	TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
	FWorldReticleParameters InReticleParams,
	bool bInIgnoreBlockingHits,
	bool bInShouldProduceTargetDataOnServer,
	bool bInUsePersistentHitResults,
	bool bInDebug,
	bool bInTraceAffectsAimPitch,
	bool bInTraceWithPawnOrientation,
	bool bInTraceFromPlayerViewPoint,
	bool bInUseAimingSpreadMod,
	float InMaxRange,
	float InBaseSpread,
	float InAimingSpreadMod,
	float InTargetingSpreadIncrement,
	float InTargetingSpreadMax,
	int32 InMaxHitResultsPerTrace,
	int32 InNumberOfTraces)
{
	ReferenceActor = InReferenceActor;
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
	bDebug = bInDebug;
	bTraceAffectsAimPitch = bInTraceAffectsAimPitch;
	bTraceWithPawnOrientation = bInTraceWithPawnOrientation;
	bTraceFromPlayerViewPoint = bInTraceFromPlayerViewPoint;
	bUseAimingSpreadMod = bInUseAimingSpreadMod;
	MaxRange = InMaxRange;
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

void AGATA_LineTrace::DoTrace(TArray<FHitResult>& HitResults, const UWorld* World,
                              const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                              const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	LineTraceWithFilter(HitResults, World, FilterHandle, Start, End, ProfileName, Params);
}

void AGATA_LineTrace::ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType,
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

		DrawDebugLineTraceMulti(GetWorld(), ViewStart, TraceEnd, DrawDebugType, true, HitResults, FLinearColor::Green,
		                        FLinearColor::Red, Duration);
	}
#endif
}

#if ENABLE_DRAW_DEBUG
// Copied from KismetTraceUtils.cpp
void AGATA_LineTrace::DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End,
                                              EDrawDebugTrace::Type DrawDebugType, bool bHit,
                                              const TArray<FHitResult>& OutHits, FLinearColor TraceColor,
                                              FLinearColor TraceHitColor, float DrawTime)
{
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		// @fixme, draw line with thickness = 2.f?
		if (bHit && OutHits.Last().bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			FVector const BlockingHitPoint = OutHits.Last().ImpactPoint;
			DrawDebugLine(World, Start, BlockingHitPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, BlockingHitPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
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
