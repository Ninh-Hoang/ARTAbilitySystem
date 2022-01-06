// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTCharacterMovementComponent.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include <AbilitySystemComponent.h>
#include "GameplayTagContainer.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

UARTCharacterMovementComponent::UARTCharacterMovementComponent()
{
	SprintSpeedMultiplier = 2.0f;
	ADSSpeedMultiplier = 0.5f;
	BlockingSpeedMultiplier = 0.0f;
	AttackingMultiplier = 0.0f;
	bUseControllerDesiredRotation = true;
	bUseGroupMovement = false;

	//flocking
	AlignmentWeight = 1.0f;
	CohesionWeight = 1.0f;
	CohesionLerp = 100.0f;
	CollisionWeight = 1.0f;
	SeparationLerp = 5.0f;
	SeparationForce = 100.0f;
	StimuliLerp = 100.0f;
	SeparationWeight = 0.8f;
	MaxMovementSpeedMultiplier = 1.5f;
	VisionRadius = 400.0f;
	CollisionDistanceLook = 400.0f;
	AlignmentComponent = FVector(0.0f, 0.0f, 0.0f);
	CohesionComponent = FVector(0.0f, 0.0f, 0.0f);
	SeparationComponent = FVector(0.0f, 0.0f, 0.0f);
	NegativeStimuliComponent = FVector(0.0f, 0.0f, 0.0f);
	PositiveStimuliComponent = FVector(0.0f, 0.0f, 0.0f);
	NegativeStimuliMaxFactor = 0.0f;
	PositiveStimuliMaxFactor = 0.0f;
	InertiaWeigh = 0.0f;
	BoidPhysicalRadius = 45.0f;
	bEnableDebugDraw = false;
	DebugRayDuration = 0.12f;
}

float UARTCharacterMovementComponent::GetMaxSpeed() const
{
	AARTCharacterBase* Owner = Cast<AARTCharacterBase>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	if (!Owner->IsAlive())
	{
		return 0.0f;
	}

	if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(
		FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun"))))
	{
		return 0.0f;
	}

	if (RequestToStartSprinting)
	{
		return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
	}

	if (RequestToStartADS)
	{
		return Owner->GetMoveSpeed() * ADSSpeedMultiplier;
	}

	if (RequestToStartBlocking)
	{
		return 0.0f;
	}

	if (RequestToStartAttacking)
	{
		return 0.0f;
	}

	return Owner->GetMoveSpeed();
}

void UARTCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	RequestToStartADS = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	RequestToStartBlocking = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;

	RequestToStartAttacking = (Flags & FSavedMove_Character::FLAG_Custom_3) != 0;
}

FNetworkPredictionData_Client* UARTCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UARTCharacterMovementComponent* MutableThis = const_cast<UARTCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FARTNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

//rotate stuffs
FRotator UARTCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	//return Owner->GetMoveSpeed();

	float YawRotateRate = 0.0f;

	AARTCharacterBase* Owner = Cast<AARTCharacterBase>(GetOwner());
	if (Owner)
	{
		YawRotateRate = Owner->GetRotateRate();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		YawRotateRate = RotationRate.Yaw;
	}

	if (!Owner->IsAlive())
	{
		YawRotateRate = 0.0f;
	}

	if (RequestToStartBlocking)
	{
		YawRotateRate = 0.0f;
	}

	if (RequestToStartAttacking)
	{
		YawRotateRate = 0.0f;
	}

	return FRotator(GetAxisDeltaRotation(RotationRate.Pitch, DeltaTime), GetAxisDeltaRotation(YawRotateRate, DeltaTime),
	                GetAxisDeltaRotation(RotationRate.Roll, DeltaTime));
}

float UARTCharacterMovementComponent::GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const
{
	return (InAxisRotationRate >= 0.f) ? (InAxisRotationRate * DeltaTime) : 360.f;
}

void UARTCharacterMovementComponent::StartSprinting()
{
	RequestToStartSprinting = true;
	IsSprinting = true;
}

void UARTCharacterMovementComponent::StopSprinting()
{
	RequestToStartSprinting = false;
	IsSprinting = false;
}

//aim stuffs
void UARTCharacterMovementComponent::StartAimDownSights()
{
	RequestToStartADS = true;
	IsAiming = true;
}

void UARTCharacterMovementComponent::StopAimDownSights()
{
	RequestToStartADS = false;
	IsAiming = false;
}

void UARTCharacterMovementComponent::StartBlocking()
{
	RequestToStartBlocking = true;
	IsBlocking = true;
}

void UARTCharacterMovementComponent::StopBlocking()
{
	RequestToStartBlocking = false;
	IsBlocking = false;
}

void UARTCharacterMovementComponent::StartAttacking()
{
	RequestToStartAttacking = true;
	IsAttacking = true;
}

void UARTCharacterMovementComponent::StopAttacking()
{
	RequestToStartAttacking = false;
	IsAttacking = false;
}

void UARTCharacterMovementComponent::FARTSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartSprinting = false;
	SavedRequestToStartADS = false;
	SavedRequestToStartBlocking = false;
	SavedRequestToStartAttacking = false;
}

uint8 UARTCharacterMovementComponent::FARTSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (SavedRequestToStartADS)
	{
		Result |= FLAG_Custom_1;
	}

	if (SavedRequestToStartBlocking)
	{
		Result |= FLAG_Custom_2;
	}

	if (SavedRequestToStartAttacking)
	{
		Result |= FLAG_Custom_3;
	}

	return Result;
}

bool UARTCharacterMovementComponent::FARTSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character,
                                                                   float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.
	if (SavedRequestToStartSprinting != ((FARTSavedMove*)NewMove.Get())->SavedRequestToStartSprinting)
	{
		return false;
	}

	if (SavedRequestToStartADS != ((FARTSavedMove*)NewMove.Get())->SavedRequestToStartADS)
	{
		return false;
	}

	if (SavedRequestToStartBlocking != ((FARTSavedMove*)NewMove.Get())->SavedRequestToStartBlocking)
	{
		return false;
	}

	if (SavedRequestToStartAttacking != ((FARTSavedMove*)NewMove.Get())->SavedRequestToStartAttacking)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UARTCharacterMovementComponent::FARTSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UARTCharacterMovementComponent* CharacterMovement = Cast<UARTCharacterMovementComponent>(
		Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
		SavedRequestToStartADS = CharacterMovement->RequestToStartADS;
		SavedRequestToStartBlocking = CharacterMovement->RequestToStartBlocking;
		SavedRequestToStartAttacking = CharacterMovement->RequestToStartAttacking;
	}
}

void UARTCharacterMovementComponent::FARTSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UARTCharacterMovementComponent* CharacterMovement = Cast<UARTCharacterMovementComponent>(
		Character->GetCharacterMovement());
	if (CharacterMovement)
	{
	}
}

UARTCharacterMovementComponent::FARTNetworkPredictionData_Client::FARTNetworkPredictionData_Client(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UARTCharacterMovementComponent::FARTNetworkPredictionData_Client::AllocateNewMove()
{
	return MakeShared<FARTSavedMove>();
}

void UARTCharacterMovementComponent::SetGroupMovementUID(int32 UID)
{
	bUseGroupMovement = UID;
}

int32 UARTCharacterMovementComponent::GetGroupMovementUID()
{
	return bUseGroupMovement;
}

void UARTCharacterMovementComponent::RequestPathMove(const FVector& MoveInput)
{
	//copied from charactermovementcomp
	FVector AdjustedMoveInput(MoveInput);

	// preserve magnitude when moving on ground/falling and requested input has Z component
	// see ConstrainInputAcceleration for details
	if (MoveInput.Z != 0.f && (IsMovingOnGround() || IsFalling()))
	{
		const float Mag = MoveInput.Size();
		AdjustedMoveInput = MoveInput.GetSafeNormal2D() * Mag;
	}

	Super::RequestPathMove(AdjustedMoveInput);
}

void UARTCharacterMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	if (MoveVelocity.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (ShouldPerformAirControlForPathFollowing())
	{
		const FVector FallVelocity = MoveVelocity.GetClampedToMaxSize(GetMaxSpeed());
		PerformAirControlForPathFollowing(FallVelocity, FallVelocity.Z);
		return;
	}

	RequestedVelocity = MoveVelocity;
	bHasRequestedVelocity = true;
	bRequestedMoveWithMaxSpeed = bForceMaxSpeed;

	if (IsMovingOnGround())
	{
		RequestedVelocity.Z = 0.0f;
	}
}

void UARTCharacterMovementComponent::SetAIConductor(UARTAIConductor* InAIConductor)
{
	if(InAIConductor) AIConductor = InAIConductor;
}

void UARTCharacterMovementComponent::RemoveFromGroup()
{
	BoidListIndex = 0;
}

void UARTCharacterMovementComponent::SetBoidGroup(int32 Key)
{
	BoidListIndex = Key;
}

int32 UARTCharacterMovementComponent::GetBoidGroupKey()
{
	return BoidListIndex;
}

bool UARTCharacterMovementComponent::ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction,
	float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed)
{
	if (bHasRequestedVelocity)
	{
		const float RequestedSpeedSquared = RequestedVelocity.SizeSquared();
		if (RequestedSpeedSquared < KINDA_SMALL_NUMBER)
		{
			return false;
		}

		// Compute requested speed from path following
		float RequestedSpeed = FMath::Sqrt(RequestedSpeedSquared);
		const FVector RequestedMoveDir = RequestedVelocity / RequestedSpeed;
		RequestedSpeed = (bRequestedMoveWithMaxSpeed ? MaxSpeed : FMath::Min(MaxSpeed, RequestedSpeed));
		
		// Compute actual requested velocity
		const FVector MoveVelocity = RequestedMoveDir * RequestedSpeed;
		
		// Compute acceleration. Use MaxAccel to limit speed increase, 1% buffer.
		FVector NewAcceleration = FVector::ZeroVector;
		const float CurrentSpeedSq = Velocity.SizeSquared();
			
		if (ShouldComputeAccelerationToReachRequestedVelocity(RequestedSpeed))
		{
			//BOIDMOVEMENT HERE
			if (bUseGroupMovement && BoidListIndex > 0 && AIConductor)
			{
				m_CurrentMoveVector = RequestedMoveDir;
				UpdateBoidNeighbourhood();
				CalculateNewMoveVector();

				const float VelSize = FMath::Sqrt(CurrentSpeedSq);
				Velocity = Velocity - (Velocity - m_NewMoveVector * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);

				// How much do we need to accelerate to get to the new velocity?
				NewAcceleration = ((m_NewMoveVector * RequestedSpeed - Velocity) / DeltaTime);
				NewAcceleration = NewAcceleration.GetClampedToMaxSize(MaxAccel);
			}
			else
			{
				// Turn in the same manner as with input acceleration.
				const float VelSize = FMath::Sqrt(CurrentSpeedSq);
				Velocity = Velocity - (Velocity - RequestedMoveDir * VelSize) * FMath::Min(DeltaTime * Friction, 1.f);

				// How much do we need to accelerate to get to the new velocity?
				NewAcceleration = ((MoveVelocity - Velocity) / DeltaTime);
				NewAcceleration = NewAcceleration.GetClampedToMaxSize(MaxAccel);
			}
		}
		else
		{
			// Just set velocity directly.
			// If decelerating we do so instantly, so we don't slide through the destination if we can't brake fast enough.
			Velocity = MoveVelocity;
		}
		
		
		// Copy to out params
		OutRequestedSpeed = RequestedSpeed;
		OutAcceleration = NewAcceleration;
		
		return true;
	}

	return false;
}

void UARTCharacterMovementComponent::UpdateBoidNeighbourhood()
{
	Neighbourhood.Empty();
	for(auto& Boid : AIConductor->GetBoidList(BoidListIndex))
	{
		//continue if the boid is the current character
		if(!Boid || GetPawnOwner() == Boid) continue;
		
		float Distance = (GetActorLocation() - Boid->GetActorLocation()).Size();
		//if in vision
		if(Distance > 0.0f && Distance < VisionRadius)
		{
			Neighbourhood.Add(Boid);
		}
	}
}

void UARTCharacterMovementComponent::CalculateNewMoveVector()
{
	ResetComponents();
	CalculateAlignmentComponentVector();
	
	if (Neighbourhood.Num() > 0)
	{
		CalculateCohesionComponentVector();
		CalculateSeparationComponentVector();
	}

	ComputeAggregationOfComponents();

	if (bEnableDebugDraw)
	{
		DebugDraw();
	}
}

void UARTCharacterMovementComponent::CalculateAlignmentComponentVector()
{
	//Compute Alignment Component Vector
	/*for (AARTCharacterAI* Boid : Neighbourhood)
	{
		AlignmentComponent += Boid->GetVelocity().GetSafeNormal(DefaultNormalizeVectorTolerance);
	}
	AlignmentComponent = (m_CurrentMoveVector + AlignmentComponent).GetSafeNormal(DefaultNormalizeVectorTolerance);*/
	AlignmentComponent  = m_CurrentMoveVector;
}

void UARTCharacterMovementComponent::CalculateCohesionComponentVector()
{
	const FVector& Location = GetActorLocation();
	for (AARTCharacterAI* Boid : Neighbourhood)
	{
		CohesionComponent += Boid->GetActorLocation()- Location;
	}

	CohesionComponent = (CohesionComponent / Neighbourhood.Num()) / CohesionLerp;
}

void UARTCharacterMovementComponent::CalculateSeparationComponentVector()
{
	const FVector& Location = GetActorLocation();

	for (AARTCharacterAI* Boid : Neighbourhood)
	{
		FVector Separation = Location - Boid->GetActorLocation();
		SeparationComponent += Separation.GetSafeNormal(DefaultNormalizeVectorTolerance)
			/ FMath::Abs(Separation.Size() - BoidPhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationComponent * SeparationForce;
	SeparationComponent += SeparationForceComponent + SeparationForceComponent *
		(SeparationLerp / Neighbourhood.Num());
}

void UARTCharacterMovementComponent::ComputeAggregationOfComponents()
{
	m_NewMoveVector = (AlignmentComponent * AlignmentWeight)
		+ (CohesionComponent * CohesionWeight)
		+ (SeparationComponent * SeparationWeight)
		+ NegativeStimuliComponent
		+ PositiveStimuliComponent;
}

void UARTCharacterMovementComponent::CorrectDirectionAgainstCollision(FVector& Direction)
{
	//check for a hit on movement
	const FVector& Location = GetActorLocation();
	static TArray<AActor*> IgnoreActor;
	FHitResult Hit(1.0f);
	if (UKismetSystemLibrary::LineTraceSingle(
		this, Location,
		Location + Direction.GetSafeNormal(DefaultNormalizeVectorTolerance) * CollisionDistanceLook,
		TraceTypeQuery1, false, IgnoreActor, EDrawDebugTrace::ForOneFrame, Hit, true))
	{
		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			Direction = FVector::VectorPlaneProject(Direction, Normal2D) * (1.f - Hit.Time) * CollisionWeight;
		}
	}
}

void UARTCharacterMovementComponent::ResetComponents()
{
	AlignmentComponent = FVector::ZeroVector;
	CohesionComponent = FVector::ZeroVector;
	SeparationComponent = FVector::ZeroVector;
	NegativeStimuliComponent = FVector::ZeroVector;
	PositiveStimuliComponent = FVector::ZeroVector;
	NegativeStimuliMaxFactor = 0.0f;
	PositiveStimuliMaxFactor = 0.0f;
}

void UARTCharacterMovementComponent::DebugDraw() const
{
	const UWorld* World = GetWorld();
	const FVector& Location = GetActorLocation();
	DrawDebugLine(World, Location,
				Location + m_CurrentMoveVector * 300.0f,
				FColor::Red, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				Location + CohesionComponent * CohesionWeight * 100.0f,
				FColor::Orange, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				Location + AlignmentComponent * AlignmentWeight * 100.0f,
				FColor::Purple, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				Location + (SeparationComponent * SeparationWeight * 100.0f),
				FColor::Blue, false, DebugRayDuration, 0, 1.0f);
}