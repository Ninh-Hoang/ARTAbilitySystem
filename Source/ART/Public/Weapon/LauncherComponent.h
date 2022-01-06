// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "LauncherComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	FM_Auto UMETA(DisplayName = "Full Auto"),
    FM_Semiauto UMETA(DisplayName = "Semiauto"),
    FM_Burst UMETA(DisplayName = "Burst"),
    FM_InterBurst UMETA(DisplayName = "Interruptible Burst"),
    FM_Manual UMETA(DisplayName = "Manual"),
    FM_Slamfire UMETA(DisplayName = "Slam Fire"),
    FM_Gatling UMETA(DisplayName = "Gatling")
};

UCLASS(Blueprintable, ClassGroup = (Custom), hidecategories = (Object, LOD, Physics, Lighting, TextureStreaming, Collision, HLOD, Mobile, VirtualTexture, ComponentReplication), editinlinenew, meta = (BlueprintSpawnableComponent))
class ART_API ULauncherComponent : public UPrimitiveComponent
{

	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	ULauncherComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//debug
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug") float DebugArrowSize = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Velocity", meta = (ToolTip = "Bullet inherits barrel velocity, only works with physics enabled or with additional velocity set")) float InheritVelocity = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Velocity", meta = (ToolTip = "Amount of recoil applied to the barrel, only works with physics enabled")) float RecoilMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Velocity", meta = (ToolTip = "Additional velocity, for use with InheritVelocity")) FVector AdditionalVelocity = FVector(0,0,0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon", meta = (ToolTip = "Additional Spread, applied on top of bullet spread")) float Spread=0.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float MuzzleVelocityMultiplierMin = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float MuzzleVelocityMultiplierMax = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float FireRateMin = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float FireRateMax = 1.0f;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Weapon") EFireMode FireMode = EFireMode::FM_Auto;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Weapon") bool ShootingBlocked;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") int BurstCount = 3;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon", meta = (ToolTip = "Automatically spin up gatling when trigger is being held down")) bool GatlingAutoSpool = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float GatlingSpoolUpTime = 1.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapon") float  GatlingSpoolDownTime = 1.0f;
	UPROPERTY(BlueprintReadWrite, Category = "Weapon") float  GatlingPhase = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo") bool CycleAmmo = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo", meta = (EditCondition = "CycleAmmo")) bool CycleAmmoUnlimited = true;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Ammo") TArray<TSubclassOf<class ABallisticProjectile>> Ammo;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Ammo", meta = (EditCondition = "CycleAmmo")) int CycleAmmoCount;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category = "Ammo", meta = (EditCondition = "CycleAmmo")) int CycleAmmoPos;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "WeaponState") TSubclassOf<class ABallisticProjectile> ChamberedBullet;
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "WeaponState") bool Shooting;
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "WeaponState") bool Spooling = false;
	UPROPERTY(BlueprintReadWrite, Category = "Weapon") float GatlingRPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "WeaponState") bool LoadNext=true;
	UPROPERTY(BlueprintReadWrite, Category = "WeaponState") float Cooldown;
	UPROPERTY(BlueprintReadWrite, Category = "WeaponState") int BurstRemaining;

	UPROPERTY(EditAnywhere, Category = "Replication") bool ReplicateVariables=true;
	UPROPERTY(EditAnywhere, Category = "Replication") bool ReplicateShotFiredEvents = true;
	UPROPERTY(EditAnywhere, Category = "Replication") bool ClientSideAim=false;
	UPROPERTY(EditAnywhere, Category = "Replication") float ClientAimUpdateFrequency = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Replication") float ClientAimDistanceLimit = 200.0f;

	FRandomStream RandomStream;

	UFUNCTION() void NextBullet();
	UFUNCTION(BlueprintPure, Category = "Ammo") int GetAmmoCount(bool CountChambered) const;
	UFUNCTION(BlueprintPure, Category = "Ammo") TArray<TSubclassOf<class ABallisticProjectile>> GetAmmo(bool CountChambered) const;
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Ammo") void SetAmmo(int count, bool UnloadChambered, bool CancelShooting, bool ManualCharge, const TArray<TSubclassOf<class ABallisticProjectile>>& NewAmmo);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ammo") void Charge();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ammo") void UnloadChambered(bool ManualCharge);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Shooting") void SwitchFireMode(EFireMode NewFireMode);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Shooting") void GatlingSpool(bool Spool);
	UFUNCTION(BlueprintCallable, Category = "Shooting") void Shoot(bool Trigger);

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IgnoredActors"), Category = "Prediction") void PredictHit(bool& Hit, FVector& HitLocation, float& HitTime, AActor*& HitActor, TArray<FVector>& Trajectory, TSubclassOf<class ABallisticProjectile> BulletClass, TArray<AActor*>IgnoredActors, float MaxTime = 10.0f, float Step = 0.1f) const;
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "IgnoredActors"), Category = "Prediction") void PredictHitFromLocation(bool &Hit, FVector& HitLocation, float& HitTime, AActor*& HitActor, TArray<FVector>& Trajectory, TSubclassOf<class ABallisticProjectile> BulletClass, FVector StartLocation, FVector AimDirection, TArray<AActor*>IgnoredActors, float MaxTime = 10.0f, float Step = 0.1f) const;
	UFUNCTION(BlueprintCallable, Category = "Prediction") void CalculateAimDirection(TSubclassOf<class ABallisticProjectile> BulletClass, FVector TargetLocation, FVector TargetVelocity, FVector& AimDirection, FVector& PredictedTargetLocation, FVector& PredictedIntersectionLocation, float& PredictedFlightTime, float& Error, float MaxTime = 10.0f, float Step = 0.1f, int NumIterations = 4) const;
	UFUNCTION(BlueprintCallable, Category = "Prediction") void CalculateAimDirectionFromLocation(TSubclassOf<class ABallisticProjectile> BulletClass, FVector StartLocation, FVector TargetLocation, FVector TargetVelocity, FVector& AimDirection, FVector& PredictedTargetLocation, FVector& PredictedIntersectionLocation, float& PredictedFlightTime, float& Error, float MaxTime = 10.0f, float Step=0.1f, int NumIterations = 4) const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Events") void InitialBulletTransform(FVector InLocation, FVector InDirection, FVector& OutLocation, FVector& OutDirection);
	UFUNCTION(BlueprintNativeEvent, Category = "Events") void ApplyRecoil(UPrimitiveComponent* Component, FVector InLocation, FVector Impulse);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShotFired);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FShotFired ShotFired;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAmmoDepleted);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FAmmoDepleted AmmoDepleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReadyToShoot);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FReadyToShoot ReadyToShoot;

#ifdef WITH_EDITOR
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
#endif
	
private:
	void SpawnBullet(AActor* Owner, FVector LocalLocation, FVector LocalAim);

	UFUNCTION(Server, Unreliable, WithValidation) void ClientAim(FVector_NetQuantize NewLocation, FVector_NetQuantizeNormal NewAim);
	UFUNCTION(Server, Reliable, WithValidation) void ShootRep(bool Trigger);
	UFUNCTION(Server, Reliable, WithValidation) void ShootRepCSA(bool Trigger, FVector_NetQuantize NewLocation, FVector_NetQuantizeNormal NewAim);

	UFUNCTION(NetMulticast, Reliable)
        void ShotFiredMulticast();

	FVector Aim;
	FVector Location;
	bool RemoteAimReceived;
	float TimeSinceAimUpdate;
	bool PredictTrace(UWorld* World, ABallisticProjectile* Bullet, FVector Start, FVector End, FVector &HitLocation, float &Time, AActor* &Actor, TArray<AActor*> IgnoredActors) const;
};
