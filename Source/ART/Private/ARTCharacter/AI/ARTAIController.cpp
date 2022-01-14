// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/AI/ARTAIController.h"

#include "AI/Order/ARTBlackboardHelper.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Order/ARTOrderHelper.h"
#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTPathFollowingComponent.h"
#include "Blueprint/ARTBlueprintFunctionLibrary.h"
#include "Framework/ARTGameState.h"

AARTAIController::AARTAIController(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UARTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    PrimaryActorTick.bCanEverTick = true;
    /*UCrowdFollowingComponent* CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent());
    CrowdFollowingComponent->SetCrowdSeparation(true);
    CrowdFollowingComponent->SetCrowdSeparationWeight(500);*/
    
    //no group at start
    GroupIndex = -1;
}

void AARTAIController::BeginPlay()
{
    Super::BeginPlay();
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

    //try to get AIConductor straight from possessed
    AIConductor = AARTGameState::GetAIConductor(GetWorld());
}

UAbilitySystemComponent* AARTAIController::GetAbilitySystemComponent() const
{
    return Cast<AARTCharacterBase>(GetPawn())->GetAbilitySystemComponent();
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

    if(bShowNavigationPath) DrawDebugNavigationPath();
}

void AARTAIController::DrawDebugNavigationPath()
{
    //make sure path following comp is valid
    UPathFollowingComponent* PathComp = GetPathFollowingComponent();
    if(!PathComp) return;

    //make sure we have a valid path
    if(!PathComp->HasValidPath()) return;
	
    const FNavPathSharedPtr PathPtr =  PathComp->GetPath();
    FNavigationPath* Path = PathPtr.Get();
    
    TArray<FNavPathPoint> PathPoints = Path->GetPathPoints();

    if(PathPoints.IsEmpty()) return;
	
    UWorld* World = GetWorld();
    for(int32 i = 0; i< PathPoints.Num(); i++)
    {
        DrawDebugPoint(World, PathPoints[i].Location, 25.f, FColor::Green, false);
        if(i == 0) continue;
		
        DrawDebugLine(World, PathPoints[i-1].Location,
            PathPoints[i].Location,
            FColor::Green,
            false,
            -1.f,
            0,
            2.f);
    }
}

void AARTAIController::SetAIConductor(UARTAIConductor* InAIConductor)
{
    if (InAIConductor) AIConductor = InAIConductor;
}

void AARTAIController::SetGroupKey(int32 InGroupIndex)
{
    GroupIndex = InGroupIndex;
}

int32 AARTAIController::GetGroupKey()
{
    return GroupIndex;
}

void AARTAIController::FindPathForMoveRequest(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query,
    FNavPathSharedPtr& OutPath) const
{
    Super::FindPathForMoveRequest(MoveRequest, Query, OutPath);
    
    TArray<FNavPathPoint> Path;
    if(AIConductor && AIConductor->FindPathForGroup(GroupIndex, Query.EndLocation, Path))
    {
         Path[0].Location = Query.StartLocation;
         Path.Last().Location = OutPath->GetPathPoints().Last().Location;
         OutPath->GetPathPoints() = Path;
         
         //If not set, goal location is original from move call and on navPathEvent::NavigationUpdated the agent will repath to original goal location
         Query.SetPathInstanceToUpdate(OutPath);
         OutPath->SetQueryData(Query);
    }
}

FPathFollowingRequestResult AARTAIController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
     return Super::MoveTo(MoveRequest, OutPath);
}
