// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_LookAtCursor.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/PlayerController.h>
#include "ART/ART.h"
#include <GameFramework/Character.h>
#include <DrawDebugHelpers.h>
#include <Kismet/KismetMathLibrary.h>

static int32 DebugAimDrawing = 0;
FAutoConsoleVariableRef CVARDebugAimingDrawing(TEXT("COOP.DebugAim"),
                                               DebugAimDrawing,
                                               TEXT("Draw Debug For Aim"),
                                               ECVF_Cheat);

UATLookAtCursor::UATLookAtCursor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsFinished = false;
}

UATLookAtCursor* UATLookAtCursor::LookAtCursor(UGameplayAbility* OwningAbility, FName TaskInstanceName,
                                               class ACharacter* Player,
                                               class UCharacterMovementComponent* MovementComponent,
                                               class APlayerController* PlayerController,
                                               UCurveFloat* OptionalInterpolationCurve)
{
	UATLookAtCursor* MyObj = NewAbilityTask<UATLookAtCursor>(OwningAbility, TaskInstanceName);
	MyObj->MovementComponent = MovementComponent;
	MyObj->PlayerController = PlayerController;
	MyObj->LerpCurve = OptionalInterpolationCurve;
	MyObj->Player = Player;
	if (MovementComponent != nullptr)
	{
		MyObj->StartRotationRate = MovementComponent->RotationRate.Yaw;
	}
	return MyObj;
}

void UATLookAtCursor::Activate()
{
	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->RotationRate.Yaw = RotationRate;
	}
}

void UATLookAtCursor::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}
	Super::TickTask(DeltaTime);

	if (PlayerController && Player)
	{
		FVector MousePosition;
		//PC->GetMousePosition(MousePosition);
		FVector WorldLocation;
		FVector WorldDirection;
		PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
		FVector ActorLocation = Player->GetActorLocation();
		FVector Intersection = FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 1000,
		                                                    ActorLocation, FVector::UpVector);
		if (DebugAimDrawing > 0 && IsLocallyControlled())
		{
			DrawDebugSphere(GetWorld(), Intersection, 10, 12, FColor::Red, false, GetWorld()->GetDeltaSeconds(), 0, 1);
			DrawDebugLine(GetWorld(), ActorLocation, Intersection, FColor::Red, false, GetWorld()->GetDeltaSeconds(), 0,
			              2);
		}

		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, Intersection);

		/*if (FMath::IsNearlyEqual(LookAtRotation.Yaw, Player->GetViewRotation().Yaw, 0.001f)) {
			bIsFinished = true;
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnTargetRotaionReached.Broadcast();
			}
			EndTask();
		}*/

		PlayerController->SetControlRotation(LookAtRotation);
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UATLookAtCursor::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate.Yaw = RotationRate;
	}
}
