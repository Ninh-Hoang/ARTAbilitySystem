// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/AI/ARTAIController.h"

#include "AI/Order/ARTBlackboardHelper.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Order/ARTOrderHelper.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"

AARTAIController::AARTAIController(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    /*UCrowdFollowingComponent* CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent());
    CrowdFollowingComponent->SetCrowdSeparation(true);
    CrowdFollowingComponent->SetCrowdSeparationWeight(500);*/
}

void AARTAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Make AI use assigned blackboard.
    UBlackboardComponent* BlackboardComponent;

    // Load assets.s
    DefaultOrder.LoadSynchronous();
    
    // Call RunBehaviorTree. This will setup the behavior tree component.
    if(IsValid(DefaultOrder.Get()))
    {
        if (UseBlackboard(CharacterBlackboardAsset, BlackboardComponent))
        {
            // Setup blackboard.
            SetBlackboardValues(FARTOrderData(DefaultOrder.Get()), InPawn->GetActorLocation());
        }
    
        // Call RunBehaviorTree. This will setup the behavior tree component.
        UBehaviorTree* BehaviorTree = UARTOrderHelper::GetBehaviorTree(DefaultOrder.Get());
        RunBehaviorTree(BehaviorTree);
    }
}

bool AARTAIController::HasOrder(TSubclassOf<UARTOrder> OrderType) const
{
    if (!VerifyBlackboard())
    {
        return false;
    }

    return Blackboard->GetValueAsClass(UARTBlackboardHelper::BLACKBOARD_KEY_ORDER_TYPE) == OrderType;
}

void AARTAIController::IssueOrder(const FARTOrderData& Order, FARTOrderCallback Callback,
                                           const FVector& HomeLocation)
{
    UBehaviorTree* BehaviorTree = UARTOrderHelper::GetBehaviorTree(Order.OrderType.Get());
    if (BehaviorTree == nullptr)
    {
        Callback.Broadcast(EARTOrderResult::FAILED);
        return;
    }

    CurrentOrderResultCallback = Callback;
    BehaviorTreeResult = EBTNodeResult::InProgress;

    SetBlackboardValues(Order, HomeLocation);

    // Stop any current orders and start over.
    ApplyOrder(Order, BehaviorTree);
}

TSoftClassPtr<UARTOrder> AARTAIController::GetStopOrder() const
{
    return DefaultOrder;
}

void AARTAIController::BehaviorTreeEnded(EBTNodeResult::Type Result)
{
    if (!VerifyBlackboard())
    {
        return;
    }

    switch (Result)
    {
        case EBTNodeResult::InProgress:
            return;
        case EBTNodeResult::Failed:
            BehaviorTreeResult = EBTNodeResult::Failed;
            return;
        case EBTNodeResult::Aborted:
            return;
        case EBTNodeResult::Succeeded:
            BehaviorTreeResult = EBTNodeResult::Succeeded;
            return;
    }
}

FVector AARTAIController::GetHomeLocation()
{
    if (!VerifyBlackboard())
    {
        return FVector::ZeroVector;
    }

    return Blackboard->GetValueAsVector(UARTBlackboardHelper::BLACKBOARD_KEY_HOME_LOCATION);
}

void AARTAIController::SetBlackboardValues(const FARTOrderData& Order, const FVector& HomeLocation)
{
    if (!VerifyBlackboard())
    {
        return;
    }

    Blackboard->SetValueAsClass(UARTBlackboardHelper::BLACKBOARD_KEY_ORDER_TYPE, Order.OrderType.Get());
    if (Order.bUseLocation)
    {
        /*Blackboard->SetValueAsVector(UARTBlackboardHelper::BLACKBOARD_KEY_LOCATION,
                                     UARTBlueprintFunctionLibrary::GetGroundLocation2D(this, Order.Location));*/
        Blackboard->SetValueAsVector(UARTBlackboardHelper::BLACKBOARD_KEY_LOCATION,
            FVector(Order.Location.X, Order.Location.Y, 0.0f));
    }
    else
    {
        Blackboard->ClearValue(UARTBlackboardHelper::BLACKBOARD_KEY_LOCATION);
    }

    Blackboard->SetValueAsObject(UARTBlackboardHelper::BLACKBOARD_KEY_TARGET, Order.Target);
    Blackboard->SetValueAsInt(UARTBlackboardHelper::BLACKBOARD_KEY_INDEX, Order.Index);
    Blackboard->SetValueAsFloat(UARTBlackboardHelper::BLACKBOARD_KEY_RANGE,
                                UARTOrderHelper::GetOrderRequiredRange(Order.OrderType, GetPawn(), Order.OrderTags, Order.Index));
    Blackboard->SetValueAsVector(UARTBlackboardHelper::BLACKBOARD_KEY_HOME_LOCATION, HomeLocation);
}

void AARTAIController::ApplyOrder(const FARTOrderData& Order, UBehaviorTree* BehaviorTree)
{
    UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
    if (BehaviorTreeComponent != nullptr && BehaviorTree != nullptr)
    {
        // Make sure to really restart the tree if the same same tree that is currently executing is passed in.
        UBehaviorTree* CurrentTree = BehaviorTreeComponent->GetRootTree();
        if (CurrentTree == BehaviorTree)
        {
            if (UARTOrderHelper::ShouldRestartBehaviourTree(Order.OrderType.Get()))
            {
                BehaviorTreeComponent->RestartTree();
            }
        }
        else
        {
            BehaviorTreeComponent->StartTree(*BehaviorTree, EBTExecutionMode::SingleRun);
        }
    }
}

bool AARTAIController::VerifyBlackboard() const
{
    if (!Blackboard)
    {
        UE_LOG(
            LogOrder, Warning,
            TEXT("Blackboard not set up for %s, can't receive orders. Check AI Controller Class and Auto Possess AI."),
            *GetPawn()->GetName());
        return false;
    }

    return true;
}

void AARTAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Blackboard == nullptr)
    {
        return;
    }

    switch (BehaviorTreeResult)
    {
        case EBTNodeResult::InProgress:
            break;
        case EBTNodeResult::Failed:
            CurrentOrderResultCallback.Broadcast(EARTOrderResult::FAILED);
            break;
        case EBTNodeResult::Aborted:
            break;
        case EBTNodeResult::Succeeded:
            CurrentOrderResultCallback.Broadcast(EARTOrderResult::SUCCEEDED);
            break;
    }

    BehaviorTreeResult = EBTNodeResult::InProgress;
}