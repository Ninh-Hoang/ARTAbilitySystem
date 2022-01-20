// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/Order/ARTOrderData.h"
#include "AI/Order/ARTOrder.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

#include "ARTAIController.generated.h"

/**
 * 
 */
class UARTOrder;

UCLASS()
class ART_API AARTAIController : public AAIController
{
	GENERATED_BODY()
public:
	AARTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Checks whether the pawn has an order of the specified type. */
	UFUNCTION(BlueprintPure)
	bool HasOrder(TSubclassOf<UARTOrder> OrderType) const;

	/** Issues this unit to obey the specified order. */
	void IssueOrder(const FARTOrderData& Order, FARTOrderCallback Callback, const FVector& HomeLocation);

	//~ Begin AActor Interface
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	UFUNCTION(Category = ART, BlueprintPure)
	TSoftClassPtr<UARTOrder> GetStopOrder() const;

	/** Called from the behavior tree to indicate that it has ended with the passed result. */
	UFUNCTION(Category = ART, BlueprintCallable)
	void BehaviorTreeEnded(EBTNodeResult::Type Result);

	/** Gets the current units home location from the black board. */
	FVector GetHomeLocation();

protected:

	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
private:
	class UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Collision object types that are used to detect attack targets. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Order", meta = (AllowPrivateAccess = true))
	TArray<TEnumAsByte<EObjectTypeQuery>> AttackTargetDetectionChannels;

	/** Collision object types that are used to find free locations. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Order", meta = (AllowPrivateAccess = true))
	TArray<TEnumAsByte<EObjectTypeQuery>> FindLocationDetectionChannels;

	/** The order to stop the character and put him in the idle state. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Order", meta = (AllowPrivateAccess = true))
	TSoftClassPtr<UARTOrder> DefaultOrder;

	/** Blackboard to use for holding all data relevant to the character AI. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Order", meta = (AllowPrivateAccess = true))
	UBlackboardData* CharacterBlackboardAsset;

	TArray<FARTOrderData> OrderQueue;

	FARTOrderCallback CurrentOrderResultCallback;

	/** Just used to cache the result of a behavior tree */
	EBTNodeResult::Type BehaviorTreeResult;

	void SetBlackboardValues(const FARTOrderData& Order, const FVector& HomeLocation);
	void ApplyOrder(const FARTOrderData& Order, UBehaviorTree* BehaviorTree);

	bool VerifyBlackboard() const;

	// RTS Group Movement
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RTS | Debug")
	bool bShowNavigationPath;
	
	UFUNCTION(BlueprintCallable, Category="RTS | Debug")
	void DrawDebugNavigationPath();

	//if 0 has no group
	int32 GroupIndex;
	
	bool bIsLeader;
	
	UPROPERTY()
	class UARTAIConductor* AIConductor;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAIPerceptionComponent* PerceptionComp;

	//Team ID, handled by Interface, different ID are hostile except +50 ID as Neutral
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ART|Character")
	uint8 TeamNumber;
	
	virtual FGenericTeamId GetGenericTeamId() const override;
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
	void SetAIConductor(UARTAIConductor* InAIConductor);
	void SetGroupKey(int32 InGroupIndex);
	int32 GetGroupKey();

	virtual void FindPathForMoveRequest(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const override;

	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr);
};
