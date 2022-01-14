// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AI/ARTAIConductor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ARTCharacterMovementComponent.generated.h"

/**
 * 
 */

UCLASS()
class ART_API UARTCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FARTSavedMove : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		///@brief Resets all saved variables.
		virtual void Clear() override;

		///@brief Store input commands in the compressed flags.
		virtual uint8 GetCompressedFlags() const override;

		///@brief This is used to check whether or not two moves can be combined into one.
		///Basically you just check to make sure that the saved variables are the same.
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

		///@brief Sets up the move before sending it to the server. 
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		///@brief Sets variables on character movement component before making a predictive correction.
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Sprint
		uint8 SavedRequestToStartSprinting : 1;

		// Aim Down Sights
		uint8 SavedRequestToStartADS : 1;

		// Aim Down Sights
		uint8 SavedRequestToStartBlocking : 1;

		// Aim Down Sights
		uint8 SavedRequestToStartAttacking : 1;
	};

	class FARTNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		FARTNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		///@brief Allocates a new copy of our custom saved move
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UARTCharacterMovementComponent();

	//movespeed stuffs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Movement")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Movement")
	float ADSSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Movement")
	float BlockingSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS Movement")
	float AttackingMultiplier;

	UPROPERTY( BlueprintReadWrite, Category = "GAS Movement")
	bool IsSprinting;
	
	UPROPERTY(BlueprintReadWrite, Category = "GAS Movement")
	bool IsAiming;
	
	UPROPERTY(BlueprintReadWrite, Category = "GAS Movement")
	bool IsBlocking;
	
	UPROPERTY(BlueprintReadWrite, Category = "GAS Movement")
	bool IsAttacking;


	uint8 RequestToStartSprinting : 1;
	uint8 RequestToStartADS : 1;
	uint8 RequestToStartBlocking :1;
	uint8 RequestToStartAttacking :1;
	
	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	//rotate stuffs
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime) const;
	
	// Sprint
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();
	
	// Aim Down Sights
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StopAimDownSights();

	//blocking
	UFUNCTION(BlueprintCallable, Category = "Blocking")
    void StartBlocking();
	UFUNCTION(BlueprintCallable, Category = "Block")
    void StopBlocking();

	//attacking
	UFUNCTION(BlueprintCallable, Category = "Blocking")
    void StartAttacking();
	UFUNCTION(BlueprintCallable, Category = "Block")
    void StopAttacking();

public:

	//group movement, flocking, steering
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group Movement")
	uint8 bUseGroupMovement:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group Movement")
	uint8 bUseContextualSteering:1;
	
	/* The weight of the Alignment vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float AlignmentWeight;

	/* The weight of the Cohesion vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CohesionWeight;

	/* The damping of the cohesion force after sum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CohesionLerp;

	/* The weight of the Collision vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CollisionWeight;

	float SeparationLerp;
	float SeparationForce;
	float StimuliLerp;

	/* The weight of the Separation vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float SeparationWeight;

	/* The maximum movement speed the Agents can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float MaxMovementSpeedMultiplier;

	/* The maximum radius at which the Agent can detect other Agents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float VisionRadius;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CollisionDistanceLook;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector AlignmentComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector CohesionComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector SeparationComponent;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector NegativeStimuliComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector PositiveStimuliComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float NegativeStimuliMaxFactor;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float PositiveStimuliMaxFactor;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float InertiaWeigh;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float BoidPhysicalRadius;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	TArray<AARTCharacterAI*> Neighbourhood;

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	TArray<class AActor*> ActorsInVision;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (Tooltip=
		"If enable components forces will be visible"))
	bool bEnableSteeringDraw;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (ClampMin=0.1f,
		ClampMax=10.0f))
	float DebugRayDuration;

	const float DefaultNormalizeVectorTolerance = 0.0001f;

	//contextual steering variables
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Contextual Steering")
	float SteerForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Contextual Steering")
	float SightRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Contextual Steering")
	int32 SightRayNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Contextual Steering")
	float ContextualLocktimer;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Contextual Steering", meta = (Tooltip=
		"If enable contextual steering force will be visible"))
	bool bEnableContextualDraw;

	TArray<FVector> SightDirections;
	TArray<float> Interest;
	TArray<float> Obstacle;
	FVector ContextualDirection;
	float ContextualLocktimerCache;

	void SetSightRayNum(int32 SightNum);
	void Ready();
	void SetInterest(FVector NavigationMoveDir);
	void SetObstacle();
	void ChooseDirection();
	
	
	void SetAIConductor(UARTAIConductor* InAIConductor);
	virtual void RemoveFromGroup();
	virtual void SetGroupKey(int32 Key);

	UFUNCTION(BlueprintPure, Category="AIBoid")
	int32 GetGroupKey();
	virtual void SetGroupMovementUID(int32 UID);
	virtual int32 GetGroupMovementUID();
	virtual void RequestPathMove(const FVector& MoveInput) override;
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	virtual bool ApplyRequestedMove(float DeltaTime, float MaxAccel, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& OutAcceleration, float& OutRequestedSpeed) override;

	virtual void BeginPlay() override;
protected:
	//if 0 has no group
	int32 BoidListIndex;

	UPROPERTY()
	UARTAIConductor* AIConductor;
	/* The movement vector (in local) this agent should move this tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector m_NewMoveVector;

	/* The movement vector (in local) this agent had last tick. */
	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	FVector m_CurrentMoveVector;

	virtual void UpdateBoidNeighbourhood();
	virtual void CalculateNewMoveVector();
	virtual void CalculateAlignmentComponentVector();
	virtual void CalculateCohesionComponentVector();
	virtual void CalculateSeparationComponentVector();
	virtual void ComputeAggregationOfComponents();
	virtual void CorrectDirectionAgainstCollision(FVector& Direction);
	virtual void ResetComponents();
	virtual void DebugDraw() const;
};
