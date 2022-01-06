// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/TargetActor/GATA_Trace.h"
#include "GATA_MeleeWeaponTrace.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMeeleeTargetDataConfirm);

UCLASS()
class ART_API AGATA_MeleeWeaponTrace : public AGATA_Trace
{
	GENERATED_BODY()

public:
	AGATA_MeleeWeaponTrace();

	UPROPERTY(BlueprintAssignable)
	FMeeleeTargetDataConfirm ValidData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	class AMeleeWeapon* MeleeWeaponActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bUsePawnSocket;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	FName MeleeSocketName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	float SocketLenght;

	class AARTCharacterBase* OwnerCharacter;

	UFUNCTION(BlueprintCallable)
	void Configure(
		UPARAM(DisplayName = "Owning Meelee Actor") class AMeleeWeapon* InMeleeWeaponActor,
		UPARAM(DisplayName = "Start Location") const FGameplayAbilityTargetingLocationInfo& InStartLocation,
		UPARAM(DisplayName = "Aiming Tag") FGameplayTag InAimingTag,
		UPARAM(DisplayName = "Aiming Removal Tag") FGameplayTag InAimingRemovalTag,
		UPARAM(DisplayName = "Trace Profile") FCollisionProfileName InTraceProfile,
		UPARAM(DisplayName = "Filter") FGameplayTargetDataFilterHandle InFilter,
		UPARAM(DisplayName = "Reticle Class") TSubclassOf<AGameplayAbilityWorldReticle> InReticleClass,
		UPARAM(DisplayName = "Reticle Params") FWorldReticleParameters InReticleParams,
		UPARAM(DisplayName = "Should Produce Target Data on Server") bool bInShouldProduceTargetDataOnServer = false,
		UPARAM(DisplayName = "Debug") bool bInDebug = false,
		UPARAM(DisplayName = "Use Pawn Socket") bool bInUsePawnSocket = false,
		UPARAM(DisplayName = "Socket Name") FName InMeleeSocketName = TEXT("MeleeSocket"),
		UPARAM(DisplayName = "Socket Length") float InSocketLenght = 0.f
	);

protected:

	TArray<FVector> PrevVecs;
	TArray<FVector> CurrVecs;

	TArray<FHitResult> CacheHitResults;

	FTimerHandle TraceTimerHandle;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> MyTargets;

	virtual void DoTrace(TArray<FHitResult>& HitResults, const UWorld* World,
	                     const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End,
	                     FName ProfileName, const FCollisionQueryParams Params) override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor) override;

	virtual void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType,
	                            float Duration = 2.0f) override;

#if ENABLE_DRAW_DEBUG
	// Util for drawing result of multi line trace from KismetTraceUtils.h
	void DrawDebugLineTraceMulti(const UWorld* World, const FVector& Start, const FVector& End,
	                             EDrawDebugTrace::Type DrawDebugType, bool bHit, const TArray<FHitResult>& OutHits,
	                             FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime);
#endif // ENABLE_DRAW_DEBUG
};
