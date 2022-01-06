// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityTask/AT_MontageDeltaCorrection.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ART/ART.h"
#include "ARTCharacter/ARTCharacterMovementComponent.h"

UAT_MontageDeltaCorrection::UAT_MontageDeltaCorrection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	LocationDeltaCorrectionActivated = false;
	RotationDeltaCorrectionActivated = false;
	UseActorTarget = false;
	LocationCorrectionIndex = 0;
	RotationCorrectionIndex = 0;

	OffSetVector = FVector(0, 0, 0);
	OffSetRotation = FRotator(0, 0, 0);

	RootEndLocation = FVector(0, 0, 0);
	ActorEndRotation = FRotator(0, 0, 0);
	ActorEndLocation = FVector(0, 0, 0);

	CurrentLocationCorrection = FVector(0, 0, 0);;
	CurrentRotationCorrection = FRotator(0, 0, 0);

	DeltaCorrectionTimeLocation = 0.f;
	RemainingDeltaTimeLocation = 0.f;

	DeltaCorrectionTimeRotation = 0.f;
	RemainingDeltaTimeRotation = 0.f;

	LocationAlpha = 0.f;
	RotationAlpha = 0.f;
}


UAT_MontageDeltaCorrection* UAT_MontageDeltaCorrection::MontageDeltaCorrect(
	UGameplayAbility* InOwningAbility,
	FName TaskInstanceName,
	USkeletalMeshComponent* InSkeletal,
	UAnimMontage* InMontageToCorrect,
	FGameplayTagContainer InLocationEventTags,
	FGameplayTagContainer InRotationEventTags,
	FVector InCorrectTarget,
	FVector InCorrectRotationTarget,
	FRotator InRotatePivotOffset,
	AActor* InActorTarget,
	float InMontagePlayRate,
	float InRootMotionTranslationScale,
	float InExpoAngle,
	bool InVelocityLimit,
	bool InRotationSpeedLimit,
	float InVelocity,
	float InRotationSpeed,
	bool InDrawBug)
{
	UAT_MontageDeltaCorrection* MyObj = NewAbilityTask<UAT_MontageDeltaCorrection>(InOwningAbility, TaskInstanceName);
	MyObj->Skeletal = InSkeletal;
	MyObj->MontageToCorrect = InMontageToCorrect;
	MyObj->LocationEventTags = InLocationEventTags;
	MyObj->RotationEventTags = InRotationEventTags;
	MyObj->CorrectTarget = InCorrectTarget;
	MyObj->CorrectRotationTarget = InCorrectRotationTarget;
	MyObj->RotatePivotOffset = InRotatePivotOffset;
	MyObj->ActorTarget = InActorTarget;
	MyObj->MontagePlayRate = InMontagePlayRate;
	MyObj->RootMotionTranslationScale = InRootMotionTranslationScale;
	MyObj->ExpoAngle = InExpoAngle;
	MyObj->DrawDebug = InDrawBug;
	MyObj->VelocityLimit = InVelocityLimit;
	MyObj->RotationSpeedLimit = InRotationSpeedLimit;
	MyObj->Velocity = InVelocity;
	MyObj->RotationSpeed = InRotationSpeed;

	//combine tags
	MyObj->CombineEventTags.AppendTags(MyObj->LocationEventTags);
	MyObj->CombineEventTags.AppendTags(MyObj->RotationEventTags);
	return MyObj;
}

void UAT_MontageDeltaCorrection::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	if (Skeletal == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s invalid Mesh"), *FString(__FUNCTION__));
		return;
	}

	RootStartLocation = Skeletal->GetBoneLocation(Skeletal->GetBoneName(0), EBoneSpaces::WorldSpace);
	MontageStartTime = GetWorld()->GetTimeSeconds();

	UARTAbilitySystemComponent* ARTAbilitySystemComponent = GetTargetASC();
	if (AbilitySystemComponent && !CombineEventTags.IsEmpty())
	{
		// Bind to event callback
		EventHandle = ARTAbilitySystemComponent->AddGameplayEventTagContainerDelegate(
			CombineEventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
				this, &UAT_MontageDeltaCorrection::OnGameplayEvent));

		if (IsValid(ActorTarget))
		{
			UseActorTarget = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("UARTAbilityTask_DeltaCorrection called on invalid AbilitySystemComponent"));
	}
	SetWaitingOnAvatar();
}

void UAT_MontageDeltaCorrection::TickTask(float DeltaTime)
{
	if (LocationDeltaCorrectionActivated)
	{
		LocationAlpha += (DeltaTime / RemainingDeltaTimeLocation);
		LocationAlpha = FMath::Clamp(LocationAlpha, 0.0f, 1.0f);

		FVector LerpOffset = FMath::InterpEaseInOut(FVector(0, 0, 0), OffSetVector, LocationAlpha, ExpoAngle);

		FVector DeltaCorrectionTick = LerpOffset - CurrentLocationCorrection;

		if (VelocityLimit) DeltaCorrectionTick = DeltaCorrectionTick.GetClampedToMaxSize(Velocity * DeltaTime); 

		GetAvatarActor()->AddActorWorldOffset(DeltaCorrectionTick);
		CurrentLocationCorrection = LerpOffset;
		//GetAvatarActor()->SetActorLocation(GetAvatarActor()->GetActorLocation() +(DeltaTime / RemainingDeltaTimeLocation) * OffSetVector );
		if (!UseActorTarget)
		{
			OffSetVector = CorrectTarget - RootEndLocation;
		}
		else
		{
			OffSetVector = ActorTarget->GetActorLocation() - RootEndLocation;
		}

		if (DrawDebug)
		{
			DrawDeltaCorrectionDebug(RootEndLocation, 50, FColor::Yellow, 0);
			DrawDeltaCorrectionDebug(RootStartLocation, 50, FColor::Red, 0);
			if(!UseActorTarget) DrawDeltaCorrectionDebug(CorrectTarget, 50, FColor::Green, 0);
			else DrawDeltaCorrectionDebug(CorrectTarget, 50, FColor::Green, 0);

			FVector CurrentRootLocation = Skeletal->GetBoneLocation(Skeletal->GetBoneName(0));
			DrawDeltaCorrectionDebug(CurrentRootLocation, 5, FColor::Blue, DeltaCorrectionTimeLocation);
		}
	}
	if (RotationDeltaCorrectionActivated)
	{
		RotationAlpha += (DeltaTime / RemainingDeltaTimeRotation);
		RotationAlpha = FMath::Clamp(RotationAlpha, 0.0f, 1.0f);

		FRotator LerpOffset = FMath::InterpEaseInOut(FRotator(0, 0, 0), OffSetRotation, RotationAlpha, ExpoAngle);
		FRotator DeltaCorrectionTick = LerpOffset - CurrentRotationCorrection;
		
		if(RotationSpeedLimit) DeltaCorrectionTick = FRotator(0,FMath::Clamp(DeltaCorrectionTick.Yaw,0.f,RotationSpeed *DeltaTime),0);

		GetAvatarActor()->AddActorWorldRotation(DeltaCorrectionTick);

		CurrentRotationCorrection = LerpOffset;
		if (!UseActorTarget)
		{
			FRotator LookAtRotation = FRotator(0.f, (CorrectRotationTarget - ActorEndLocation).Rotation().Yaw, 0.f);
			OffSetRotation = (LookAtRotation - ActorEndRotation).GetDenormalized();
		}
		else
		{
			FRotator LookAtRotation = FRotator(0.f, (ActorTarget->GetActorLocation() - ActorEndLocation).Rotation().Yaw,
			                                   0.f);
			OffSetRotation = (LookAtRotation - ActorEndRotation).GetDenormalized();;
		}
	}
}

void UAT_MontageDeltaCorrection::OnDestroy(bool AbilityEnded)
{
	if (UARTAbilitySystemComponent* ARTAbilitySystemComponent = GetTargetASC()) ARTAbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(CombineEventTags, EventHandle);
	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	if(UARTCharacterMovementComponent* MoveComp = GetTargetMovementComp()) MoveComp->SetMovementMode(EMovementMode::MOVE_Flying);

	Super::OnDestroy(AbilityEnded);
}

void UAT_MontageDeltaCorrection::ExternalSetCorrectionLocation(FVector InLocation, AActor* InActor)
{
	if (IsValid(InActor))
	{
		UseActorTarget = true;
		ActorTarget = InActor;
	}
	else
	{
		UseActorTarget = false;
		CorrectTarget = InLocation;
	}
}

void UAT_MontageDeltaCorrection::ExternalSetCorrectionRotation(FVector InRotationTarget)
{
	CorrectRotationTarget = InRotationTarget;
}

void UAT_MontageDeltaCorrection::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (EventTag.MatchesAny(LocationEventTags))
	{
		//DELTA CORRECTION FOR LOCATION INIT
		LocationDeltaCorrectionActivated = true;

		if(UARTCharacterMovementComponent* MoveComp = GetTargetMovementComp()) MoveComp->SetMovementMode(EMovementMode::MOVE_Flying);

		//interpolation
		LocationAlpha = 0.0f;
		CurrentLocationCorrection = FVector(0, 0, 0);

		//set time
		DeltaCorrectionTimeLocation = Payload->EventMagnitude / MontagePlayRate;
		RemainingDeltaTimeLocation = DeltaCorrectionTimeLocation;

		float StartTrackPosition = GetWorld()->GetTimeSeconds() - MontageStartTime;
		float EndTrackPosition = StartTrackPosition + RemainingDeltaTimeLocation;

		//calculate root end 
		FTransform RootExtract = MontageToCorrect->ExtractRootMotionFromTrackRange(
			StartTrackPosition, EndTrackPosition);

		FTransform RootWorldTransform = Skeletal->ConvertLocalRootMotionToWorld(RootExtract);
		FVector CurrentRootLocation = Skeletal->GetBoneLocation(Skeletal->GetBoneName(0));

		RootEndLocation = CurrentRootLocation + RootWorldTransform.GetTranslation();

		//calculate offset distance
		if (!UseActorTarget) OffSetVector = CorrectTarget - RootEndLocation ;
		else OffSetVector = ActorTarget->GetActorLocation() - RootEndLocation;

		OnLocationCorrectStart.Broadcast(LocationCorrectionIndex);

		GetWorld()->GetTimerManager().SetTimer(LocationDeltaTimerHandle,
		                                       this,
		                                       &UAT_MontageDeltaCorrection::EndLocationCorrection,
		                                       RemainingDeltaTimeLocation,
		                                       false);
	}
	else
	{
		/*
		* DELTA CORRECTION FOR ROTATION INIT
		*/
		RotationDeltaCorrectionActivated = true;

		//interpolation
		RotationAlpha = 0.0f;
		CurrentRotationCorrection = FRotator(0, 0, 0);

		//set time
		DeltaCorrectionTimeRotation = Payload->EventMagnitude / MontagePlayRate;
		RemainingDeltaTimeRotation = DeltaCorrectionTimeRotation;

		float StartTrackPosition = GetWorld()->GetTimeSeconds() - MontageStartTime;
		float EndTrackPosition = StartTrackPosition + RemainingDeltaTimeRotation;

		//calculate root end
		FTransform RootExtract = MontageToCorrect->ExtractRootMotionFromTrackRange(
			StartTrackPosition, EndTrackPosition);

		FTransform RootWorldTransform = Skeletal->ConvertLocalRootMotionToWorld(RootExtract);
		FRotator CurrentRootRotation = Skeletal->GetBoneQuaternion(Skeletal->GetBoneName(0)).Rotator();
		FVector CurrentRootLocation = Skeletal->GetBoneLocation(Skeletal->GetBoneName(0));

		//calculate offset rotation
		ActorEndRotation = GetAvatarActor()->GetActorRotation() + FRotator(
			0, RootExtract.GetRotation().Rotator().Yaw, 0);
		ActorEndLocation = GetAvatarActor()->GetActorLocation() + RootWorldTransform.GetTranslation();
		ActorEndRotation += RotatePivotOffset;

		FRotator LookAtRotation;
		if (!UseActorTarget) LookAtRotation = FRotator(0.f, (CorrectRotationTarget - ActorEndLocation).Rotation().Yaw, 0.f);	
		else LookAtRotation = FRotator(0.f, (ActorTarget->GetActorLocation() - ActorEndLocation).Rotation().Yaw, 0.f);
		
		OffSetRotation = (LookAtRotation - ActorEndRotation).GetDenormalized();

		if (DrawDebug)
		{
			FVector StartTrace = GetAvatarActor()->GetActorLocation();
			FVector EndTrace = StartTrace + GetAvatarActor()->GetActorForwardVector() * 300;
			FVector EndTraceCorrect = EndTrace;
			//EndTraceCorrect = CorrectRotation.RotateVector(EndTraceCorrect);
		}
		UE_LOG(LogTemp, Warning, TEXT("Look At Rot: %s"), *CurrentRootRotation.ToString());

		OnRotationCorrectStart.Broadcast(RotationCorrectionIndex);

		GetWorld()->GetTimerManager().SetTimer(RotationDeltaTimerHandle,
		                                       this,
		                                       &UAT_MontageDeltaCorrection::EndRotationCorrection,
		                                       RemainingDeltaTimeRotation,
		                                       false);
	}
}

UARTAbilitySystemComponent* UAT_MontageDeltaCorrection::GetTargetASC()
{
	return Cast<UARTAbilitySystemComponent>(AbilitySystemComponent);
}

UARTCharacterMovementComponent* UAT_MontageDeltaCorrection::GetTargetMovementComp()
{
	return Cast<UARTCharacterMovementComponent>(Ability->GetActorInfo().MovementComponent);
}

void UAT_MontageDeltaCorrection::DrawDeltaCorrectionDebug(FVector Location, float Radius, FColor Color, float Time)
{
#if ENABLE_DRAW_DEBUG
	if (DrawDebug)
	{
		DrawDebugSphere(GetWorld(), Location, Radius, 12, Color, false, Time, 0, 2);
	}
#endif
}

void UAT_MontageDeltaCorrection::EndLocationCorrection()
{
	LocationDeltaCorrectionActivated = false;
	if(UARTCharacterMovementComponent* MoveComp = GetTargetMovementComp()) MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	OnLocationCorrectEnd.Broadcast(LocationCorrectionIndex);
	LocationCorrectionIndex++;
}

void UAT_MontageDeltaCorrection::EndRotationCorrection()
{
	RotationDeltaCorrectionActivated = false;
	OnRotationCorrectEnd.Broadcast(RotationCorrectionIndex);
	RotationCorrectionIndex++;
}
