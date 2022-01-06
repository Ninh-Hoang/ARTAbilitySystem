// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BallisticProjectile.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABallisticProjectile::ABallisticProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_PrePhysics);
	SetActorEnableCollision(false);
}

// Called when the game starts or when spawned
void ABallisticProjectile::BeginPlay()
{
	Super::BeginPlay();

	//recycle from pool
	if(!IsRecycled){
		Super::BeginPlay();
		IsRecycled = true;
	}
	else{
		ReceiveBeginPlay();
	}
	
	if (SafeLaunch) {
		OwnerSafe = true;
	}
	
	if (DoFirstStepImmediately) {
		float DeltaTime = GetWorld()->GetDeltaSeconds();

		if (RandomFirstStepDelta) {
			DeltaTime *= RandomStream.FRand();
		};

		if (FixedStep) {
			Step(FixedStepSeconds);
		}
		else {
			Step(DeltaTime);
		}
	}
}

// Called every frame
void ABallisticProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (FixedStep) {
		AccumulatedDelta += DeltaTime;
    		
		while (AccumulatedDelta >= FixedStepSeconds) {
			Step(FixedStepSeconds);
			AccumulatedDelta -= FixedStepSeconds;
		}
	}

	else {
		Step(DeltaTime);
	}
}

void ABallisticProjectile::Step(float DeltaTime)
{
	FVector start = GetActorLocation();
	bool sendUpdate = false;

	if (Retrace && CanRetrace) {
		//time travel
		float remainingTime = LastTraceDelta;
		int remainingSteps = MaxTracesPerStep;
		FVector PreviousVelocity = LastTracePrevVelocity;
		SetActorLocation(LastTraceStart);
		Velocity = LastTraceVelocity;

		do {
			if (RetraceOnAnotherChannel) {
				remainingTime = Trace(GetActorLocation(),
                    PreviousVelocity,
                    remainingTime,
                    RetraceChannel);
			}
			else {
				remainingTime = Trace(GetActorLocation(),
                    PreviousVelocity,
                    remainingTime,
                    TraceChannel);
			}
			PreviousVelocity = Velocity;
			remainingSteps -= 1;
			if (remainingTime > 0.0f) { sendUpdate = true; };
		} while (remainingTime > 0.0f && remainingSteps > 0);
	}
	CanRetrace = false;

	FVector PreviousVelocity = Velocity;
	Velocity = UpdateVelocity(GetWorld(), GetActorLocation(), Velocity, DeltaTime);

	//trace
	float remainingTime = DeltaTime;
	int remainingSteps = MaxTracesPerStep;
	do {
		remainingTime = Trace(GetActorLocation(), 
            PreviousVelocity, 
            remainingTime,
            TraceChannel
        );
		PreviousVelocity = Velocity;
		remainingSteps -= 1;
		if (remainingTime > 0.0f) { sendUpdate = true; };
	} while (remainingTime > 0.0f && remainingSteps > 0);

	if (sendUpdate)
	{
		if (ReliableReplication) {
			VelocityChangeBroadcastReliable(UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(),GetActorLocation()), Velocity);
		}
		else {
			VelocityChangeBroadcast(UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), GetActorLocation()), Velocity);
		}
	}

	if(SafeDelay <= 0.0f){
		OwnerSafe = false;
	}
	else {
		SafeDelay -= DeltaTime;
	}

	if (RotateActor) {
		FRotator NewRot = UKismetMathLibrary::MakeRotFromX(Velocity);
		NewRot.Roll = GetActorRotation().Roll;
		SetActorRotation(NewRot);
	}
}


void ABallisticProjectile::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);
	LastTraceStart += InOffset;
}

void ABallisticProjectile::LifeSpanExpired()
{
	Deactivate();
}

void ABallisticProjectile::SpawnWithExactVelocity(TSubclassOf<ABallisticProjectile> BulletClass, AActor* BulletOwner,
	APawn* BulletInstigator, FVector BulletLocation, FVector BulletVelocity)
{
	if (BulletClass != nullptr && BulletOwner != nullptr) {
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = BulletOwner;
		spawnParams.Instigator = BulletInstigator;

		ABallisticProjectile* Default = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());

		FTransform Transform;
		Transform.SetLocation(BulletLocation);
		Transform.SetScale3D(Default->GetActorScale());

		if (Default->RotateActor) {
			FRotator Rotation = UKismetMathLibrary::MakeRotFromX(BulletVelocity);
			if (Default->RotateRandomRoll) Rotation.Add(0, 0, Default->RandomStream.FRandRange(-180.0f, 180.0f));
			Transform.SetRotation(Rotation.Quaternion());
		}
		else {
			Transform.SetRotation(FQuat(1, 0, 0, 1));
		}

		if (!Default->Shotgun) {
			ABallisticProjectile* bullet = SpawnOrReactivate(BulletOwner->GetWorld(), BulletClass, Transform, BulletVelocity, BulletOwner, BulletInstigator);
		}
		else {
			for (int i = 0; i < Default->ShotCount; i++) {
				float Vel = BulletVelocity.Size()*Default->RandomStream.FRandRange(1.0 - Default->ShotVelocitySpread, 1.0 + Default->ShotVelocitySpread);
				FVector SubmunitionVelocity = Default->RandomStream.VRandCone(BulletVelocity, Default->ShotSpread)*Vel;
				ABallisticProjectile* bullet = SpawnOrReactivate(BulletOwner->GetWorld(), BulletClass, Transform, SubmunitionVelocity, BulletOwner, BulletInstigator);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cannot spawn bullet - invalid class or owner"));
	}
}

void ABallisticProjectile::Spawn(TSubclassOf<ABallisticProjectile> BulletClass, AActor* BulletOwner, APawn* BulletInstigator,
	FVector BulletLocation, FVector BulletVelocity)
{
}

void ABallisticProjectile::VelocityChangeBroadcast_Implementation(FVector_NetQuantize NewLocation, FVector NewVelocity)
{
	if (!HasAuthority()) {
		FVector RebasedLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);
		OnTrajectoryUpdateReceived(RebasedLocation, Velocity, NewVelocity);
		SetActorLocation(RebasedLocation);
		Velocity = NewVelocity;
		CanRetrace = false;
	}
}

void ABallisticProjectile::VelocityChangeBroadcastReliable_Implementation(FVector_NetQuantize NewLocation,
	FVector NewVelocity)
{
	if (!HasAuthority()) {
		FVector RebasedLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);
		OnTrajectoryUpdateReceived(RebasedLocation, Velocity, NewVelocity);
		SetActorLocation(RebasedLocation);
		Velocity = NewVelocity;
		CanRetrace = false;
	}
}

FVector ABallisticProjectile::UpdateVelocity_Implementation(UWorld* World, FVector Location, FVector PreviousVelocity,
	float DeltaTime) const
{
	FVector NewVelocity = PreviousVelocity;

	//TODO these valud are fixed for now
	//airDensity
	float air = 1.21f;
	float speedOfSound = 34300.0f;

	//air = GetAirDensity(World, Location);
	//speedOfSound = GetSpeedOfSound(World, Location);

	//gravity
	if (!OverrideGravity) {
		NewVelocity += FVector(0, 0, World->GetGravityZ())*DeltaTime;
	}
	else {
		NewVelocity += Gravity*DeltaTime;
	};

	//drag
	//TODO WIND and WORLDSCALE IS FIXED
	FVector relVel = (NewVelocity - FVector(0.0f, 0.0f, 0.0f)/*GetWind(World, Location*/);
	float speed = relVel.Size();
	float mach = speed / speedOfSound;
	float profile = FMath::Pow(Diameter / 200.0f, 2.0f)*3.141592f;
	float drag = GetCurveValue(MachDragCurve, mach, 0.25f)*FMath::Pow(speed / 100.0f, 2.0f)*profile*air*FormFactor*50.0f;
	NewVelocity -= relVel.GetSafeNormal() * drag / Mass * DeltaTime / 1.0f/*WorldScale*/;

	return NewVelocity;
}

bool ABallisticProjectile::CollisionFilter_Implementation(FHitResult HitResult) const
{
	return true;
}

void ABallisticProjectile::ReactivationBroadcast_Implementation(FVector_NetQuantize NewLocation, FVector NewVelocity,
	AActor* BulletOwner, APawn* BulletInstigator)
{
}

void ABallisticProjectile::DeactivationBroadcast_Implementation()
{
	if (!HasAuthority()) {
		this->DeativateToPool();
	}
}

void ABallisticProjectile::OnImpact_Implementation(bool Ricochet, bool PassedThrough, FVector Location,
                                                   FVector IncomingVelocity, FVector Normal, FVector ExitLocation, FVector ExitVelocity, FVector Impulse,
                                                   float PenetrationDepth, AActor* Actor, USceneComponent* Component, FName BoneName, UPhysicalMaterial* PhysMaterial,
                                                   FHitResult HitResult)
{
	return;
}

void ABallisticProjectile::OnNetPredictedImpact_Implementation(bool Ricochet, bool PassedThrough, FVector Location,
	FVector IncomingVelocity, FVector Normal, FVector ExitLocation, FVector ExitVelocity, FVector Impulse,
	float PenetrationDepth, AActor* Actor, USceneComponent* Component, FName BoneName, UPhysicalMaterial* PhysMaterial,
	FHitResult HitResult)
{
	return;
}

void ABallisticProjectile::Deactivate()
{
	//server only
	if (!HasAuthority()) { return; }
	this->DeativateToPool();
	DeactivationBroadcast();
}

ABallisticProjectile* ABallisticProjectile::GetFromPool(UWorld* World, UClass* BulletClass)
{
	ABallisticProjectile* Pool = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());

	if (Pool) {
		//find first of correct class;
		bool CleanupRequired=false;

		int32 FoundIndex = Pool->Pooled.IndexOfByPredicate(
            [&](auto InItem) {
            if (InItem.IsValid() && InItem->GetWorld() == World) {
                return true;
            }
            else {
                CleanupRequired = true;
                return false;
            }
        });

		TWeakObjectPtr<ABallisticProjectile> Found = nullptr;
		if (FoundIndex != INDEX_NONE) {
			Found = Pool->Pooled[FoundIndex];
			Pool->Pooled.RemoveAtSwap(FoundIndex);
		}

		if (CleanupRequired) {
#ifdef WITH_EDITOR
			if (Pool->DebugPooling) {
				GEngine->AddOnScreenDebugMessage(2, 2, FColor::White, TEXT("Invalid reference in pool, cleaning up"));
			}
#endif
			Pool->Pooled.RemoveAll([&](auto InItem) {
                if (InItem.IsValid() && InItem->GetWorld() == World) {
                    return false;
                }
                else {
                    return true;
                }
            });
		}

		return(Found.Get());
	}
	else {
		return nullptr;
	}
}

ABallisticProjectile* ABallisticProjectile::SpawnOrReactivate(UWorld* World, TSubclassOf<ABallisticProjectile> BulletClass,
	const FTransform& Transform, FVector BulletVelocity, AActor* BulletOwner, APawn* BulletInstigator)
{
	ABallisticProjectile* bullet;

	ABallisticProjectile* Recycled = GetFromPool(World, BulletClass); 

	if (Recycled) {
		ABallisticProjectile* Default = Cast<ABallisticProjectile>(BulletClass->GetDefaultObject());

		Recycled->SetOwner(BulletOwner);
		Recycled->SetInstigator(BulletInstigator);
		Recycled->SetActorTransform(Transform);
		Recycled->Velocity = BulletVelocity;
		Recycled->SetActorHiddenInGame(Default->IsHidden());
		Recycled->SetActorTickEnabled(true);
		Recycled->CanRetrace = false;
		Recycled->IgnoredActors = Default->IgnoredActors;
		Recycled->SafeDelay = Default->SafeDelay;
		Recycled->SetLifeSpan(Default->InitialLifeSpan);
		if (!Recycled->HasActorBegunPlay()){ Recycled->BeginPlay(); }
		Recycled->ReactivationBroadcast(UGameplayStatics::RebaseLocalOriginOntoZero(Recycled->GetWorld(), Transform.GetLocation()), BulletVelocity, BulletOwner, BulletInstigator);
#ifdef WITH_EDITOR
		if (Recycled->DebugPooling) {
			GEngine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Recycling pooled bullet"));
		}
#endif
		return Recycled;
	}
	else {
		bullet = Cast<ABallisticProjectile>(World->SpawnActorDeferred<ABallisticProjectile>(BulletClass, Transform, BulletOwner, BulletInstigator));
		bullet->RandomStream.GenerateNewSeed();
		bullet->Velocity = BulletVelocity;
		UGameplayStatics::FinishSpawningActor(bullet, Transform);
#ifdef WITH_EDITOR
		if (bullet->DebugPooling) {
			GEngine->AddOnScreenDebugMessage(0, 2, FColor::Orange, TEXT("Spawning new bullet"));
		}
#endif
		return bullet;
	}
}

void ABallisticProjectile::DeativateToPool()
{
	ABallisticProjectile* Pool = Cast<ABallisticProjectile>(GetClass()->GetDefaultObject());

	if (Pool && EnablePooling) {
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
		Pool->Pooled.Add(this);
		if (HasActorBegunPlay()) {
			EndPlay(EEndPlayReason::RemovedFromWorld);
		}

		if (Pool->Pooled.Num() > MaxPoolSize) {
			ABallisticProjectile* Oldest = (Pool->Pooled[0].Get());
			Pool->Pooled.RemoveAtSwap(0);
			if (Oldest) { Oldest->Destroy(); }
		}

#ifdef WITH_EDITOR
		if (DebugPooling) {
			GEngine->AddOnScreenDebugMessage(2, 2, FColor::White, FString("Bullet pooled: ") + FString::FromInt(Pool->Pooled.Num()));
		}
#endif
	}
	else {
		Destroy();
	}
}

float ABallisticProjectile::Trace(FVector start, FVector PreviousVelocity, float delta,
	TEnumAsByte<ECollisionChannel> CollisionChannel)
{
	bool Hit;
	FHitResult HitResult;
	TArray<FHitResult> Results;

	FCollisionResponseParams ResponseParameters;

	FCollisionQueryParams CollisionParameters;
	CollisionParameters.bTraceComplex = TraceComplex;
	CollisionParameters.bReturnPhysicalMaterial = true;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActors(IgnoredActors);
	CollisionParameters.bReturnFaceIndex = true;

	if (OwnerSafe) {
		CollisionParameters.AddIgnoredActors(GetSafeLaunchIgnoredActors(GetOwner()));
	}

	FVector TraceDistance = (PreviousVelocity + Velocity)*0.5*delta;
	
	GetWorld()->LineTraceMultiByChannel(Results, start, start + TraceDistance, CollisionChannel, CollisionParameters, ResponseParameters);
	if (Results.Num() > 0) {
		HitResult = FilterHits(Results, Hit);
	}
	else { Hit = false; }
	
	if (Hit)
	{
		//Reduce velocity
		Velocity = FMath::Lerp(PreviousVelocity, Velocity, HitResult.Time);

		bool Ricochet = false;
		bool Penetration = false;
		FVector exitLoc;
		FVector exitNormal;
		FVector NewVelocity = Velocity;

		//material mods
		/*bool neverPenetrate = false;
		bool neverRicochet = false;
		float penDepthMultiplier = 1.0f;
		float penNormalization = PenetrationNormalization;
		float penNormalizationGrazing = PenetrationNormalizationGrazing;
		float penEnterSpread = PenetrationEntryAngleSpread;
		float penExitSpread = PenetrationExitAngleSpread;
		float ricProbMultiplier = 1.0f;
		float ricRestitution = RicochetRestitution;
		float ricFriction = RicochetFriction;
		float ricSpread = RicochetSpread;
		EPenTraceType PenTraceType = DefaultPenTraceType;*/

		UPhysicalMaterial* PhysMaterial = HitResult.PhysMaterial.Get();

		if (PhysMaterial) {
			//material response modifiers
			/*if (MaterialResponseMap != nullptr) {
				FEBMaterialResponseMapEntry* ResponseEntry = MaterialResponseMap->Map.Find(PhysMaterial);
				if (ResponseEntry != nullptr) {
					neverPenetrate = ResponseEntry->NeverPenetrate;
					neverRicochet = ResponseEntry->NeverRicochet;
					PenTraceType = ResponseEntry->PenTraceType;

					penDepthMultiplier = ResponseEntry->PenetrationDepthMultiplier;
					penNormalization = PenetrationNormalization + ResponseEntry->PenetrationNormalization;
					penNormalizationGrazing = PenetrationNormalizationGrazing + ResponseEntry->PenetrationNormalizationGrazing;
					penEnterSpread = PenetrationEntryAngleSpread + ResponseEntry->PenetrationEntryAngleSpread;
					penExitSpread = PenetrationExitAngleSpread + ResponseEntry->PenetrationExitAngleSpread;

					ricProbMultiplier = ResponseEntry->RicochetProbabilityMultiplier;
					ricRestitution = FMath::Lerp(RicochetRestitution, ResponseEntry->RicochetRestitution, ResponseEntry->RicochetRestitutionInfluence);
					ricFriction = FMath::Lerp(RicochetFriction, ResponseEntry->RicochetFriction, ResponseEntry->RicochetFrictionInfluence);
					ricSpread = RicochetSpread + ResponseEntry->RicochetSpread;
				}	
			}

			if (MaterialDensityControlsPenetrationDepth) {
				penDepthMultiplier /= PhysMaterial->Density;
			}

			if (MaterialRestitutionControlsRicochet) {
				RicochetRestitution *= PhysMaterial->Restitution;
			}*/
			
			float dot = FVector::DotProduct(Velocity.GetSafeNormal(), HitResult.Normal) + 1.0f;
			FVector cross = FVector::CrossProduct(Velocity.GetSafeNormal(), HitResult.Normal);
			FVector flat = HitResult.Normal.RotateAngleAxis(-90.0f, cross);

#ifdef WITH_EDITOR
			if (DebugEnabled) {
				FColor DebugColor = FColor::MakeRedToGreenColorFromScalar(Velocity.Size() / MuzzleVelocityMax);
				DrawDebugLine(GetWorld(), start, HitResult.Location, DebugColor, false, DebugTrailTime, 0, DebugTrailWidth);
			};
#endif
		}

		//despawn
		if ((Velocity.Size() < DespawnVelocity) | (!Ricochet && !Penetration)){
			Deactivate();
		}
		CanRetrace = false;
	}

	else {
		//prepare for time travel
		if (Retrace) {
			CanRetrace = true;
			LastTraceStart = start;
			LastTraceDelta = delta;
			LastTracePrevVelocity = PreviousVelocity;
			LastTraceVelocity = Velocity;
		}

		SetActorLocation(start + TraceDistance);
		HitResult.Time = 1.0f;

		OnTrace(start, GetActorLocation());

#ifdef WITH_EDITOR
		if (DebugEnabled) {
			FLinearColor Color = GetDebugColor(Velocity.Size() / ((MuzzleVelocityMin + MuzzleVelocityMax)*0.5f));
			DrawDebugLine(GetWorld(), start, start + TraceDistance, Color.ToFColor(true), false, DebugTrailTime, 0, 0);
		}
	}
#endif

	return delta*(1.0f - HitResult.Time);
}

TArray<AActor*> ABallisticProjectile::GetAttachedActorsRecursive(AActor* Actor, uint16 Depth) const
{
	TArray<AActor*> Attached;
	Actor->GetAttachedActors(Attached);

	TArray<AActor*> AttachedRecursive;
	for (AActor* ActorRecursive : Attached) {
		AttachedRecursive += GetAttachedActorsRecursive(ActorRecursive);
	}
	Attached += AttachedRecursive;
	return Attached;
}

float ABallisticProjectile::GetCurveValue(const UCurveFloat* curve, float in, float deflt) const
{
	if (curve == nullptr) return deflt;
	return curve->GetFloatValue(in);
}

FHitResult ABallisticProjectile::FilterHits(TArray<FHitResult> Results, bool& hit) const
{
	TArray<FHitResult> OutResults;

	for (FHitResult Result : Results) {
		if (Result.bBlockingHit) {

			hit = true;
			return Result;
		}else{
			if (CollisionFilter(Result)) {
				hit = true;
				return Result;
			}
		}
	}

	hit = false;
	return FHitResult(); //blank
}

TArray<AActor*> ABallisticProjectile::GetSafeLaunchIgnoredActors(AActor* ProjectileOwner) const
{
	TArray<AActor*> Results = SafeLaunchIgnoredActors;

	Results.Add(ProjectileOwner);

	if (SafeLaunchIgnoreAttachParent && ProjectileOwner) {
		AActor* AttachedRoot = ProjectileOwner;

		while (true) { //find attachment root
			AActor* AttachedTo;
			AttachedTo = AttachedRoot->GetAttachParentActor();

			if (AttachedTo) {
				Results.Add(AttachedTo);
				AttachedRoot = AttachedTo;
			}
			else break;
		}
		Results.Add(AttachedRoot);
		if (SafeLaunchIgnoreAllAttached) Results.Append(GetAttachedActorsRecursive(AttachedRoot));
	}

	return Results;
}

void ABallisticProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABallisticProjectile, Velocity, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ABallisticProjectile, RandomStream, COND_InitialOnly);
}

