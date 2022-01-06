// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/LauncherComponent.h"
#include "Weapon/BallisticProjectile.h"

#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"

#define REPOWNERONLY false

ULauncherComponent::ULauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bHiddenInGame = true;
	bAutoActivate = true;
	SetIsReplicatedByDefault(ReplicateVariables);

	RandomStream.GenerateNewSeed();

	GatlingRPS = FireRateMin;
}

void ULauncherComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ClientSideAim)
	{
		if (GetOwner()->GetRemoteRole() == ROLE_Authority)
		{
			TimeSinceAimUpdate += DeltaTime;
			if ((TimeSinceAimUpdate >= 1.0f / ClientAimUpdateFrequency) && Shooting)
			{
				Aim = GetComponentTransform().GetUnitAxis(EAxis::X);
				Location = GetComponentTransform().GetLocation();
				ClientAim(UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), Location), Aim);
				TimeSinceAimUpdate = FMath::Fmod(TimeSinceAimUpdate, 1.0f / ClientAimUpdateFrequency);
			};
		}
		else
		{
			if (!RemoteAimReceived)
			{
				Aim = GetComponentTransform().GetUnitAxis(EAxis::X);
				Location = GetComponentTransform().GetLocation();
			}
			else
			{
				FVector LocOffset = (Location - GetComponentLocation());
				if (LocOffset.Size() > ClientAimDistanceLimit)
				{
					//lag or cheater???
					Location = GetComponentLocation() + LocOffset.GetSafeNormal() * ClientAimDistanceLimit;
				}
			}
		}
	}
	else
	{
		Aim = GetComponentTransform().GetUnitAxis(EAxis::X);
		Location = GetComponentTransform().GetLocation();
	}
	//Only server can tick
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		float RemainingDelta;

		if (FireMode == EFireMode::FM_Gatling)
		{
			if (Spooling || (GatlingAutoSpool && Shooting))
			{
				GatlingRPS = FMath::Lerp(GatlingRPS, FireRateMax, FMath::Min(GatlingSpoolUpTime * DeltaTime, 1.0f));
			}
			else
			{
				GatlingRPS = FMath::Lerp(GatlingRPS, FireRateMin, FMath::Min(GatlingSpoolUpTime * DeltaTime, 1.0f));
			}
			GatlingPhase += GatlingRPS * DeltaTime;
			for (int i = 1; i <= GatlingPhase; i++)
			{
				if (Cooldown <= 0.0f && LoadNext)
				{
					NextBullet();
				}

				if (Shooting && ChamberedBullet != nullptr && (!ShootingBlocked))
				{
					SpawnBullet(GetOwner(), Location, Aim);
				}
			}
			GatlingPhase = FMath::Fmod(GatlingPhase, 1.0f);
		}
		else
		{
			RemainingDelta = DeltaTime;
			do
			{
				float step = FMath::Min(Cooldown, RemainingDelta);

				Cooldown -= step;

				RemainingDelta -= step;

				if (Cooldown <= 0.0f && LoadNext)
				{
					NextBullet();
				}

				//shoot when ready
				if (Shooting && ChamberedBullet != nullptr && (!ShootingBlocked))
				{
					if (BurstRemaining > 0 || (FireMode != EFireMode::FM_Burst && FireMode != EFireMode::FM_InterBurst))
					{
						SpawnBullet(GetOwner(), Location, Aim);
					}
					else
					{
						Shooting = false;
					}
				}
			}
			while (RemainingDelta > 0 && Cooldown > 0);
		}
	}
}

void ULauncherComponent::NextBullet()
{
	if (Ammo.Num() > 0 && ChamberedBullet == nullptr && (CycleAmmoCount > 0 || CycleAmmoUnlimited || (!CycleAmmo)))
	{
		//cycle ammo
		if (CycleAmmo)
		{
			if (CycleAmmoPos >= Ammo.Num()) { CycleAmmoPos = 0; }
			ChamberedBullet = Ammo[CycleAmmoPos];
			CycleAmmoPos++;

			if (!CycleAmmoUnlimited)
			{
				CycleAmmoCount--;
			}
		}
		else
		{
			ChamberedBullet = Ammo[0];
			Ammo.RemoveAt(0, 1, true);
		}

		ReadyToShoot.Broadcast();
	}
	else
	{
		AmmoDepleted.Broadcast();
	}
}

void ULauncherComponent::SpawnBullet(AActor* Owner, FVector InLocation, FVector InAim)
{
	TSubclassOf<class ABallisticProjectile> BulletClass = ChamberedBullet;

	if (BulletClass != nullptr)
	{
		FVector OutLocation;
		FVector OutAim;

		InitialBulletTransform(InLocation, InAim, OutLocation, OutAim);

		ABallisticProjectile* Default = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());

		OutAim = RandomStream.VRandCone(OutAim, Spread + Default->Spread);
		float BulletVelocity = FMath::Lerp(MuzzleVelocityMultiplierMin * Default->MuzzleVelocityMin,
		                                   MuzzleVelocityMultiplierMax * Default->MuzzleVelocityMax,
		                                   RandomStream.FRand());
		FVector Velocity = OutAim * BulletVelocity;

		//get parent physics body
		UPrimitiveComponent* parent = Cast<UPrimitiveComponent>(GetAttachParent());
		Velocity += AdditionalVelocity;

		if (parent != nullptr)
		{
			if (parent->IsSimulatingPhysics())
			{
				Velocity += parent->GetPhysicsLinearVelocityAtPoint(OutLocation) * InheritVelocity;
			}

			if (Default->Shotgun)
			{
				ApplyRecoil(parent, OutLocation, -Velocity * Default->Mass * RecoilMultiplier * Default->ShotCount);
			}
			else
			{
				ApplyRecoil(parent, OutLocation, -Velocity * Default->Mass * RecoilMultiplier);
			}
		}

		ABallisticProjectile::SpawnWithExactVelocity(BulletClass, Owner, Owner->GetInstigator(), OutLocation, Velocity);

		//spend ammo
		ChamberedBullet = nullptr;
		if (FireMode != EFireMode::FM_Gatling)
		{
			Cooldown = 1.0f / FMath::Lerp(FireRateMin, FireRateMax, RandomStream.FRand());
		}

		//fire modes
		switch (FireMode)
		{
		case EFireMode::FM_Auto:
			LoadNext = true;
			break;

		case EFireMode::FM_Burst:
			LoadNext = true;
			break;

		case EFireMode::FM_InterBurst:
			LoadNext = true;
			break;

		case EFireMode::FM_Semiauto:
			Shooting = false;
			LoadNext = true;
			break;

		case EFireMode::FM_Manual:
			Shooting = false;
			LoadNext = false;
			break;

		case EFireMode::FM_Slamfire:
			LoadNext = false;
			break;

		case EFireMode::FM_Gatling:
			LoadNext = true;
			break;
		};

		if (BurstRemaining > 0)
		{
			BurstRemaining--;
		}

		if (ReplicateShotFiredEvents)
		{
			ShotFiredMulticast();
		}
		else
		{
			ShotFired.Broadcast();
		}
	}
}

int ULauncherComponent::GetAmmoCount(bool CountChambered) const
{
	int remainingAmmo;
	if (CycleAmmo) {
		remainingAmmo = CycleAmmoCount;
	}
	else {
		remainingAmmo = Ammo.Num();
	};

	if (CountChambered) {
		if (ChamberedBullet != nullptr) {
			remainingAmmo++;
		};
	};

	return remainingAmmo;
}

TArray<TSubclassOf<ABallisticProjectile>> ULauncherComponent::GetAmmo(bool CountChambered) const
{
	if (!CountChambered || ChamberedBullet == nullptr) {
		return Ammo;
	}
	else {
		TArray<TSubclassOf<class ABallisticProjectile>> RetAmmo;
		RetAmmo.Add(ChamberedBullet);
		RetAmmo.Append(Ammo);
		return RetAmmo;
	};
}

void ULauncherComponent::SetAmmo(int Count, bool UnloadChambered, bool CancelShooting, bool ManualCharge,
                                 const TArray<TSubclassOf<ABallisticProjectile>>& NewAmmo)
{
	Ammo = NewAmmo;

	CycleAmmoCount = Count;

	if (UnloadChambered) {
		ChamberedBullet = nullptr;
	};

	if (CancelShooting) {
		BurstRemaining = 0;
		Shooting = false;
	};

	if (ManualCharge) {
		LoadNext = false;
	};
}

void ULauncherComponent::Charge_Implementation()
{
	LoadNext = true;
}

bool ULauncherComponent::Charge_Validate()
{
	return true;
}

void ULauncherComponent::UnloadChambered_Implementation(bool ManualCharge)
{
	ChamberedBullet = nullptr;

	if (ManualCharge) {
		LoadNext = false;
	};
}

bool ULauncherComponent::UnloadChambered_Validate(bool ManualCharge)
{
	return true;
}

void ULauncherComponent::SwitchFireMode_Implementation(EFireMode NewFireMode)
{
	FireMode = NewFireMode;
}

bool ULauncherComponent::SwitchFireMode_Validate(EFireMode NewFireMode)
{
	return true;
}

void ULauncherComponent::GatlingSpool_Implementation(bool Spool)
{
	Spooling = Spool;
}

bool ULauncherComponent::GatlingSpool_Validate(bool Spool)
{
	return true;
}

void ULauncherComponent::Shoot(bool Trigger)
{
	if (ClientSideAim && GetOwner()->GetRemoteRole() == ROLE_Authority && Trigger) {
		Aim = GetComponentTransform().GetUnitAxis(EAxis::X);
		Location = GetComponentTransform().GetLocation();
		ShootRepCSA(Trigger, UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), Location), Aim);
	}
	else {
		ShootRep(Trigger);
	}
}

void ULauncherComponent::PredictHit(bool& Hit, FVector& HitLocation, float& HitTime, AActor*& HitActor,
                                    TArray<FVector>& Trajectory, TSubclassOf<ABallisticProjectile> BulletClass,
                                    TArray<AActor*> IgnoredActors, float MaxTime,
                                    float Step) const
{
	FVector StartLocation = GetComponentLocation();
	FVector AimDirection = GetComponentQuat().GetForwardVector();
	PredictHitFromLocation(Hit, HitLocation, HitTime, HitActor, Trajectory, BulletClass, StartLocation, AimDirection, IgnoredActors, MaxTime, Step);
}

void ULauncherComponent::PredictHitFromLocation(bool& Hit, FVector& HitLocation, float& HitTime, AActor*& HitActor,
                                                TArray<FVector>& Trajectory, TSubclassOf<ABallisticProjectile> BulletClass,
                                                FVector StartLocation, FVector AimDirection,
                                                TArray<AActor*> IgnoredActors, float MaxTime, float Step) const
{
	if (!BulletClass->IsValidLowLevel()) {
		UE_LOG(LogTemp, Warning, TEXT("PredictHit - invalid bullet class"));
		return;
	}

	float Time = 0;
	Trajectory = TArray<FVector>();

	FVector CurrentLocation = StartLocation;
	ABallisticProjectile* Bullet = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());
	FVector Velocity = AimDirection.GetSafeNormal()*(FMath::Lerp(MuzzleVelocityMultiplierMin, MuzzleVelocityMultiplierMax, 0.5)*FMath::Lerp(Bullet->MuzzleVelocityMin, Bullet->MuzzleVelocityMax, 0.5));

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());

	Velocity += AdditionalVelocity;

	if (Parent != nullptr) {
		if (Parent->IsSimulatingPhysics()) {
			Velocity += Parent->GetPhysicsLinearVelocityAtPoint(CurrentLocation)*InheritVelocity;
		}
	}

	while (Time < MaxTime) {
		FVector PreviousVelocity = Velocity;
		Velocity = Bullet->UpdateVelocity(GetWorld(), CurrentLocation, Velocity, Step);
		float TraceHitTime;
		Hit = ULauncherComponent::PredictTrace(GetWorld(), Bullet, CurrentLocation, CurrentLocation + FMath::Lerp(PreviousVelocity, Velocity, 0.5f)*Step, HitLocation,TraceHitTime, HitActor, IgnoredActors);
		if (Hit) {
			Trajectory.Add(HitLocation);
			HitTime = Time+(TraceHitTime*Step);
			return;
		}
		else {
			Trajectory.Add(CurrentLocation);
			CurrentLocation += FMath::Lerp(PreviousVelocity, Velocity, 0.5f)*Step;
			Time += Step;
		}
	}

	Hit = false;
	HitTime = MaxTime;
	HitLocation = CurrentLocation;
	HitActor = nullptr;
}

void ULauncherComponent::CalculateAimDirection(TSubclassOf<ABallisticProjectile> BulletClass, FVector TargetLocation,
                                               FVector TargetVelocity, FVector& AimDirection,
                                               FVector& PredictedTargetLocation,
                                               FVector& PredictedIntersectionLocation, float& PredictedFlightTime,
                                               float& Error, float MaxTime, float Step,
                                               int NumIterations) const
{
	FVector StartLocation = GetComponentLocation();
	CalculateAimDirectionFromLocation(BulletClass, StartLocation, TargetLocation, TargetVelocity, AimDirection, PredictedTargetLocation, PredictedIntersectionLocation, PredictedFlightTime, Error, MaxTime, Step, NumIterations);
}

void ULauncherComponent::CalculateAimDirectionFromLocation(TSubclassOf<ABallisticProjectile> BulletClass, FVector StartLocation,
                                                           FVector TargetLocation, FVector TargetVelocity,
                                                           FVector& AimDirection, FVector& PredictedTargetLocation,
                                                           FVector& PredictedIntersectionLocation,
                                                           float& PredictedFlightTime, float& Error, float MaxTime,
                                                           float Step,
                                                           int NumIterations) const
{
	if (!BulletClass->IsValidLowLevel()) {
		UE_LOG(LogTemp, Warning, TEXT("CalculateAimDirection - invalid bullet class"));
		return;
	}

	ABallisticProjectile* bullet = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());

	FVector AddVelocity = AdditionalVelocity;
	UPrimitiveComponent* parent = Cast<UPrimitiveComponent>(GetAttachParent());
	if (parent != nullptr) {
		if (parent->IsSimulatingPhysics()) {
			AddVelocity += parent->GetPhysicsLinearVelocityAtPoint(StartLocation) * InheritVelocity;
		}
	}


	FVector InitialAimDirection = (TargetLocation - StartLocation).GetSafeNormal(); //initial prediction
	AimDirection = InitialAimDirection;
	FVector PreviousAimDirection = AimDirection;

	for (int Iteration = 0; Iteration < NumIterations; Iteration++) {
		FVector CurrentBulletLocation = StartLocation;
		FVector Velocity = (AimDirection * (FMath::Lerp(MuzzleVelocityMultiplierMin, MuzzleVelocityMultiplierMax, 0.5) * FMath::Lerp(bullet->MuzzleVelocityMin, bullet->MuzzleVelocityMax, 0.5))) + AddVelocity;
		bool hit = 0;
		for (float time = 0; time <= MaxTime; time += Step) {
			FVector PreviousVelocity = Velocity;
			Velocity = bullet->UpdateVelocity(GetWorld(), CurrentBulletLocation, Velocity, Step);

			FVector TraceVector = ((((PreviousVelocity + Velocity) * 0.5) - TargetVelocity) * Step);
			FVector TraceEndLocation = CurrentBulletLocation + TraceVector;
			FVector IntersectionPoint;

			hit = FMath::SegmentPlaneIntersection(CurrentBulletLocation - TraceVector, TraceEndLocation, FPlane(TargetLocation, InitialAimDirection), IntersectionPoint); //actual hit test

			if (hit) {
				PredictedIntersectionLocation = IntersectionPoint;
				FQuat AimCorrection = FQuat::FindBetween((IntersectionPoint - StartLocation), (TargetLocation - StartLocation));
				AimDirection = AimCorrection.RotateVector(AimDirection).GetSafeNormal();
				Error = (IntersectionPoint - TargetLocation).Size();

				float AdditionalFlightTime = (FVector(CurrentBulletLocation - IntersectionPoint).Size() / TraceVector.Size()) * Step;
				PredictedFlightTime = time + AdditionalFlightTime;
				PredictedTargetLocation = TargetLocation + TargetVelocity * AdditionalFlightTime;

				break;
			}

			//no hit, keep going
			CurrentBulletLocation = TraceEndLocation;
		}
		if (!hit) {
			Error = 99999999999999999.0f;
			return; //no solution
		}
	}
}

void ULauncherComponent::InitialBulletTransform_Implementation(FVector InLocation, FVector InDirection,
                                                               FVector& OutLocation, FVector& OutDirection)
{
	OutLocation = InLocation;
	OutDirection = InDirection;
}

void ULauncherComponent::ApplyRecoil_Implementation(UPrimitiveComponent* Component, FVector InLocation, FVector Impulse)
{
	if (Component->IsSimulatingPhysics()) {
		Component->AddImpulseAtLocation(Impulse, InLocation);
	}
}

#ifdef WITH_EDITOR
#include "PrimitiveSceneProxy.h"
FPrimitiveSceneProxy* ULauncherComponent::CreateSceneProxy()
{
	{
		class FBarrelProxy : public FPrimitiveSceneProxy
		{
		public:
			FBarrelProxy(ULauncherComponent* InComponent) : FPrimitiveSceneProxy(InComponent)
			{
				bWillEverBeLit = false;
				Component = InComponent;
			}

			virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
			{
				QUICK_SCOPE_CYCLE_COUNTER(STAT_BarrelSceneProxy_GetDynamicMeshElements);

				const FMatrix& Transform = GetLocalToWorld();

				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (VisibilityMap && (1 << ViewIndex))
					{
						const FSceneView* View = Views[ViewIndex];
						const FLinearColor DrawColor = GetViewSelectionColor(FColor::Green, *View, IsSelected(), IsHovered(), true, IsIndividuallySelected());

						FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
						DrawDirectionalArrow(PDI, Transform, DrawColor, Component->DebugArrowSize, Component->DebugArrowSize*0.1f, 16, Component->DebugArrowSize*0.01f);
					}
				}
			}

			virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
			{
				const bool bProxyVisible = IsSelected();

				FPrimitiveViewRelevance Result;
				Result.bDrawRelevance = (IsShown(View));
				Result.bDynamicRelevance = true;
				Result.bShadowRelevance = false;
				Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
				return Result;
			}
			virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
			uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }
			virtual SIZE_T GetTypeHash() const override { return 0; }

		private:
			ULauncherComponent* Component;
		};

		return new FBarrelProxy(this);
	}
}
#endif

void ULauncherComponent::ClientAim_Implementation(FVector_NetQuantize NewLocation, FVector_NetQuantizeNormal NewAim)
{
	Location = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(),NewLocation);
	Aim = NewAim;
	RemoteAimReceived = true;
}

bool ULauncherComponent::ClientAim_Validate(FVector_NetQuantize NewLocation, FVector_NetQuantizeNormal NewAim)
{
	return true;
}

void ULauncherComponent::ShootRep_Implementation(bool Trigger)
{
	if (Trigger) {
		if (FireMode == EFireMode::FM_Burst || FireMode == EFireMode::FM_InterBurst) {
			BurstRemaining = BurstCount;
		};
		Shooting = true;
	}
	else {
		//burst cannot be interrupted
		if (FireMode != EFireMode::FM_Burst || BurstRemaining<=0) {
			Shooting = false;
		}
	}
}

bool ULauncherComponent::ShootRep_Validate(bool Trigger)
{
	return true;
}

void ULauncherComponent::ShootRepCSA_Implementation(bool Trigger, FVector_NetQuantize NewLocation,
                                                    FVector_NetQuantizeNormal NewAim)
{
	Location = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);
	Aim = NewAim;
	RemoteAimReceived = true;

	if (Trigger) {
		if (FireMode == EFireMode::FM_Burst || FireMode == EFireMode::FM_InterBurst) {
			BurstRemaining = BurstCount;
		};
		Shooting = true;
	}
	else {
		//burst cannot be interrupted
		if (FireMode != EFireMode::FM_Burst || BurstRemaining <= 0) {
			Shooting = false;
		}
	}
}

bool ULauncherComponent::ShootRepCSA_Validate(bool Trigger, FVector_NetQuantize NewLocation,
                                              FVector_NetQuantizeNormal NewAim)
{
	return true;
}

void ULauncherComponent::ShotFiredMulticast_Implementation()
{
	ShotFired.Broadcast();
}

bool ULauncherComponent::PredictTrace(UWorld* World, ABallisticProjectile* Bullet, FVector Start, FVector End,
                                      FVector& HitLocation, float& Time, AActor*& Actor,
                                      TArray<AActor*> IgnoredActors) const
{
	FHitResult Result;

	FCollisionResponseParams ResponseParams;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = Bullet->TraceComplex;
	QueryParams.bReturnPhysicalMaterial = true;

	if (Bullet->SafeLaunch) {
		QueryParams.AddIgnoredActor(GetOwner());
	}

	QueryParams.AddIgnoredActors(IgnoredActors);
		
	bool Hit = World->LineTraceSingleByChannel(Result, Start, End, Bullet->TraceChannel, QueryParams, ResponseParams);

	if (Hit) {
		Time = Result.Time;
		Actor = Result.GetActor();
		HitLocation = Result.Location;
	}
	return Hit;
}

void ULauncherComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

#if REPOWNERONLY
	DOREPLIFETIME_CONDITION((ULauncherComponent, FireMode, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, CycleAmmoCount, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, CycleAmmoPos, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, Ammo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, ChamberedBullet, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, Shooting, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, ShootingBlocked, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION((ULauncherComponent, Spooling, COND_OwnerOnly);
#else
	DOREPLIFETIME(ULauncherComponent , FireMode);
	DOREPLIFETIME(ULauncherComponent , CycleAmmoCount);
	DOREPLIFETIME(ULauncherComponent , CycleAmmoPos);
	DOREPLIFETIME(ULauncherComponent , Ammo);
	DOREPLIFETIME(ULauncherComponent , ChamberedBullet);
	DOREPLIFETIME(ULauncherComponent , Shooting);
	DOREPLIFETIME(ULauncherComponent , ShootingBlocked);
	DOREPLIFETIME(ULauncherComponent , Spooling);
#endif
}
