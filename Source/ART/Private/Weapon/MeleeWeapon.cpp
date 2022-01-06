// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/MeleeWeapon.h"
#include <Engine/World.h>
#include <DrawDebugHelpers.h>
#include <ARTCharacter/ARTCharacterBase.h>

// Sets default values
AMeleeWeapon::AMeleeWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

AMeleeWeapon::~AMeleeWeapon()
{
}

// Called when the game starts or when spawned
void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMeleeWeapon::StartTrace()
{
	//reset all array
	CurrVecs.Reset();
	MyTargets.Empty();

	UWorld* World = GetWorld();
	//initialize vector values
	for (int i = WeaponStartLength; i <= WeaponTail; i += 10)
	{
		CurrVecs.Add(GetActorLocation() + i * GetActorUpVector());
	}

	//set timer tick
	World->GetTimerManager().SetTimer(TraceTimerHandle, this, &AMeleeWeapon::CheckCollision, World->GetDeltaSeconds(),
	                                  true, 0);
}

void AMeleeWeapon::EndTrace()
{
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
}

void AMeleeWeapon::CheckCollision()
{
	PrevVecs = CurrVecs;
	for (int i = WeaponStartLength; i <= WeaponTail; i += 20)
	{
		const int32 size_ = (i - WeaponStartLength) / 10;

		CurrVecs[size_] = GetActorLocation() + i * GetActorUpVector();

		DrawTraceLine(PrevVecs[size_], CurrVecs[size_], bEnableDrawTraceLine);
	}
}

bool AMeleeWeapon::TryExcludeActor(AActor* HitActor)
{
	if (MyTargets.Contains(HitActor))
	{
		return false;
	}
	MyTargets.Add(HitActor);
	return true;
}

void AMeleeWeapon::DrawTraceLine(FVector prevVec_, FVector currVec_, bool bDrawTraceLine)
{
	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bIsHit = GetWorld()->LineTraceMultiByChannel(Hits, prevVec_, currVec_, ECC_Pawn, QueryParams);
	if (bIsHit)
	{
		if (bDrawTraceLine)
			DrawDebugLine(GetWorld(), prevVec_, currVec_, FColor::Green, false, 2.f, 0, 1.f);

		for (const auto& Hit : Hits)
		{
			AARTCharacterBase* TargetPawn = Cast<AARTCharacterBase>(Hit.GetActor());
			/*if (TargetPawn && TryExcludeActor(TargetPawn))
			{
			    ApplyEventBackToGA(TargetPawn, Hit);
			}*/
		}
	}
	else if (bDrawTraceLine)
		DrawDebugLine(GetWorld(), prevVec_, currVec_, FColor::Red, false, 2.f, 0, 1.f);
}
