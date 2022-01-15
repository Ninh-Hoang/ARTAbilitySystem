// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Order/ARTAutoOrderComponent.h"
#include "ART/ART.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Order/ARTAutoOrderProvider.h"
#include "AI/Order/ARTOrderHelper.h"
#include "AI/Order/ARTOrderComponent.h"

DECLARE_CYCLE_STAT(TEXT("AART - Auto Order Target Acquisition"), STAT_ARTAutoOrderTargetAcquisition, STATGROUP_Order);


UARTAutoOrderComponent::UARTAutoOrderComponent()
{
    //SetIsReplicated(true);
    PrimaryComponentTick.bCanEverTick = true;
    SetComponentTickInterval(0.1f);
    bCheckAutoOrders = false;
}

void UARTAutoOrderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UARTAutoOrderComponent, HumanPlayerAutoOrderStates);
}

bool UARTAutoOrderComponent::IsHumanPlayerAutoOrder(const FARTOrderTypeWithIndex& Order)
{
    for (int32 i = 0; i < Orders.Num(); ++i)
    {
        if (Order == Orders[i])
        {
            return HumanPlayerAutoOrders[i];
        }
    }

    return false;
}

void UARTAutoOrderComponent::SetHumanPlayerAutoOrderState(const FARTOrderTypeWithIndex& Order, bool bEnable)
{
    for (int32 i = 0; i < Orders.Num(); ++i)
    {
        if (Order == Orders[i])
        {
            HumanPlayerAutoOrderStates[i] = bEnable;
            if (bEnable)
            {
                bCheckAutoOrders = true;
            }

            break;
        }
    }
}

bool UARTAutoOrderComponent::GetHumanPlayerAutoOrderState(const FARTOrderTypeWithIndex& Order)
{
    for (int32 i = 0; i < Orders.Num(); ++i)
    {
        if (Order == Orders[i])
        {
            return HumanPlayerAutoOrderStates[i];
        }
    }

    return false;
}

void UARTAutoOrderComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();

    if (!IsValid(Owner))
    {
        return;
    }

    UARTOrderComponent* OrderComponent = Owner->FindComponentByClass<UARTOrderComponent>();
    if (OrderComponent == nullptr)
    {
        return;
    }

    // Listen for the appropriate order to enable auto orders.
    OrderComponent->OnOrderChanged.AddDynamic(this, &UARTAutoOrderComponent::OnOrderChanged);

    // Find all auto orders.
    UClass* ProviderInterfaceClass = UARTAutoOrderProvider::StaticClass();
    if (Owner->GetClass()->ImplementsInterface(ProviderInterfaceClass))
    {
        IARTAutoOrderProvider::Execute_GetAutoOrders(Owner, Orders);
        
        if(IARTAutoOrderProvider* AutoOrderProvider = Cast<IARTAutoOrderProvider>(Owner))
        {
            FOnAutoOrderUpdate* OnAutoOrderAdded = AutoOrderProvider->GetAutoOrderAddDelegate();
            FOnAutoOrderUpdate* OnAutoOrderRemoved= AutoOrderProvider->GetAutoOrderRemoveDelegate();
            if(OnAutoOrderAdded) (*OnAutoOrderAdded).AddDynamic(this, &UARTAutoOrderComponent::OnAutoOrderAdded);
            if(OnAutoOrderRemoved) (*OnAutoOrderRemoved).AddDynamic(this, &UARTAutoOrderComponent::OnAutoOrderRemoved);
        }
    }

    for (UActorComponent* Component : Owner->GetComponents())
    {
        if (Component->GetClass()->ImplementsInterface(ProviderInterfaceClass))
        {
            IARTAutoOrderProvider::Execute_GetAutoOrders(Component, Orders);

            if(IARTAutoOrderProvider* AutoOrderProvider = Cast<IARTAutoOrderProvider>(Component))
            {
                FOnAutoOrderUpdate* OnAutoOrderAdded = AutoOrderProvider->GetAutoOrderAddDelegate();
                FOnAutoOrderUpdate* OnAutoOrderRemoved= AutoOrderProvider->GetAutoOrderRemoveDelegate();
                if(OnAutoOrderAdded) (*OnAutoOrderAdded).AddDynamic(this, &UARTAutoOrderComponent::OnAutoOrderAdded);
                if(OnAutoOrderRemoved) (*OnAutoOrderRemoved).AddDynamic(this, &UARTAutoOrderComponent::OnAutoOrderRemoved);
            }
        }
    }

    if (Orders.Num() == 0)
    {
        return;
    }

    HumanPlayerAutoOrders.AddDefaulted(Orders.Num());

    // Only certain abilities are auto cast abilities for human players.
    for (int32 i = 0; i < Orders.Num(); ++i)
    {
        HumanPlayerAutoOrders[i] = UARTOrderHelper::IsHumanPlayerAutoOrder(Orders[i].OrderType, Owner, Orders[i].OrderTags, Orders[i].Index);
    }

    // Calculating states and AI data is done on server-side, only.
    if (!Owner->HasAuthority())
    {
        return;
    }

    HumanPlayerAutoOrderStates.AddDefaulted(Orders.Num());
    AIPlayerAutoOrders.AddDefaulted(Orders.Num());

    bool bHasAutoCastOrders = false;

    for (int32 i = 0; i < Orders.Num(); ++i)
    {
        if (HumanPlayerAutoOrders[i])
        {
            bool bInitialEnabled =
                UARTOrderHelper::GetHumanPlayerAutoOrderInitialState(Orders[i].OrderType, Owner, Orders[i].OrderTags,Orders[i].Index);
            bHasAutoCastOrders |= bInitialEnabled;
            HumanPlayerAutoOrderStates[i] = bInitialEnabled;
        }

        AIPlayerAutoOrders[i] = UARTOrderHelper::IsAIPlayerAutoOrder(Orders[i].OrderType, Owner, Orders[i].OrderTags,Orders[i].Index);
        bHasAutoCastOrders |= AIPlayerAutoOrders[i];
    }

    // NOTE(np): In A Year Of Rain, AI units have auto cast enabled by default.
    //if (OwnerComponent->GetPlayerOwner() == nullptr || UARTUtilities::IsAIUnit(Owner))
    //{
    //    bHasAutoCastOrders = true;
    //}

    // NOTE(np): In A Year Of Rain, units can change their owner at runtime (e.g. rescued units in story campaign).
    //UARTOwnerComponent* OwnerComponent = Owner->FindComponentByClass<UARTOwnerComponent>();
    //if (OwnerComponent != nullptr)
    //{
    //    // Listen for owner changes.
    //    OwnerComponent->OnOwnerChanged.AddDynamic(this, &UARTAutoOrderComponent::OnOwnerChanged);
    //}

    FARTOrderData CurrentOrder = OrderComponent->GetCurrentOrderData();
    if (bHasAutoCastOrders && UARTOrderHelper::AreAutoOrdersAllowedDuringOrder(CurrentOrder.OrderType, GetOwner(), CurrentOrder.OrderTags, CurrentOrder.Index))
    {
        bCheckAutoOrders = true;
    }

    // NOTE(np): A Year Of Rain registers the actor with a manager for bucketed updates here, to improve performance.
    //// Register actor with game.
    //AARTGameMode* GameMode = Cast<AARTGameMode>(UGameplayStatics::GetGameMode(this));

    //if (IsValid(GameMode))
    //{
    //    GameMode->AddAutoOrderActor(Owner);
    //}
}


void UARTAutoOrderComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CheckAutoOrders();
}

void UARTAutoOrderComponent::CheckAutoOrders()
{
    if (!bCheckAutoOrders)
    {
        return;
    }

    AActor* Owner = GetOwner();

    //TODO: this is determine if pawn is player or AI, need a custom one to determine is player team or hostile team
    bool bIsAIUnit = Cast<ACharacter>(Owner)->IsPlayerControlled() ? false : true;
    //bool bIsAIUnit = false;

    {
        SCOPE_CYCLE_COUNTER(STAT_ARTAutoOrderTargetAcquisition);

        for (int32 i = 0; i < Orders.Num(); ++i)
        {
            if ((bIsAIUnit && AIPlayerAutoOrders[i]) ||
                (!bIsAIUnit && HumanPlayerAutoOrders[i] && HumanPlayerAutoOrderStates[i]))
            {
                FARTOrderTypeWithIndex Order = Orders[i];
                if (IssueAutoOrder(Order))
                {
                    break;
                }
            }
        }
    }
}

void UARTAutoOrderComponent::OnOrderChanged(const FARTOrderData& NewOrder)
{
    bCheckAutoOrders = UARTOrderHelper::AreAutoOrdersAllowedDuringOrder(NewOrder.OrderType, GetOwner(), NewOrder.OrderTags, NewOrder.Index);
}

void UARTAutoOrderComponent::OnOwnerChanged(APlayerState* PreviousOwner, APlayerState* NewOwner)
{
    if (!IsValid(NewOwner))
    {
        return;
    }

    UARTOrderComponent* OrderComponent = GetOwner()->FindComponentByClass<UARTOrderComponent>();
    if (OrderComponent == nullptr)
    {
        return;
    }

    bool bHasAutoCastOrders = false;

    // For AI players
    if (NewOwner->IsABot())
    {
        // Every ability is an auto cast ability for AI players.
        bHasAutoCastOrders = Orders.Num() > 0;
    }

    // For human players
    else
    {
        // Only certain abilities are auto cast abilities for human players.
        for (int32 i = 0; i < Orders.Num(); ++i)
        {
            if (HumanPlayerAutoOrders[i] && HumanPlayerAutoOrderStates[i])
            {
                bHasAutoCastOrders = true;
                break;
            }
        }
    }

    if (bHasAutoCastOrders)
    {
        FARTOrderData CurrentOrder = OrderComponent->GetCurrentOrderData();
        bCheckAutoOrders = UARTOrderHelper::AreAutoOrdersAllowedDuringOrder(CurrentOrder.OrderType, GetOwner(), CurrentOrder.OrderTags, CurrentOrder.Index);
    }
}

bool UARTAutoOrderComponent::IssueAutoOrder(const FARTOrderTypeWithIndex& Order)
{
    float AcquisitionRadius = GetAcquisitionRadius(Order);

    AActor* Owner = GetOwner();
    if (!UARTOrderHelper::CanObeyOrder(Order.OrderType, GetOwner(), Order.OrderTags, Order.Index))
    {
        return false;
    }

    UARTOrderComponent* OrderComponent = Owner->FindComponentByClass<UARTOrderComponent>();
    if (OrderComponent == nullptr)
    {
        return false;
    }

    EARTTargetType TargetType = UARTOrderHelper::GetTargetType(Order.OrderType, Owner, Order.OrderTags, Order.Index);
    switch (TargetType)
    {
        case EARTTargetType::NONE:
        {
            // TODO: Orders with no target type (this basically resolves to 'self') are only issued when their is an
            // enemy nearby. This might not always be the case (e.g self only heal).
            /*if (UARTOrderHelper::IsEnemyInAcquisitionRadius(Owner, AcquisitionRadius))
            {*/
                OrderComponent->InsertOrderBeforeCurrentOrder(FARTOrderData(Order.OrderType, Order.Index));
                return true;
            //}
        }
        break;
        case EARTTargetType::ACTOR:
        case EARTTargetType::LOCATION:
        case EARTTargetType::DIRECTION:
        {
            float Score;
            AActor* Target =
                UARTOrderHelper::FindTargetForOrder(Order.OrderType, Owner, Order.OrderTags, Order.Index, AcquisitionRadius, Score);
            if (Target != nullptr)
            {
                OrderComponent->InsertOrderBeforeCurrentOrder(
                    FARTOrderData(Order.OrderType, Order.Index, Order.OrderTags, Target, FVector2D(Target->GetActorLocation())));
                return true;
            }
        }
        break;
        case EARTTargetType::PASSIVE:
            break;
        default:
            break;
    }

    return false;
}

float UARTAutoOrderComponent::GetAcquisitionRadius(const FARTOrderTypeWithIndex& Order)
{
    AActor* Owner = GetOwner();

    float AcquisitionRadius = 0.0f;
    if (UARTOrderHelper::GetAcquisitionRadiusOverride(Order.OrderType, Owner, Order.OrderTags, Order.Index, AcquisitionRadius))
    {
        return AcquisitionRadius;
    }
    // NOTE(np): A Year Of Rain returns the default acquisition radius of the units here.

    /*UARTAttackComponent* AttackComponent = Owner->FindComponentByClass<UARTAttackComponent>();
    if (AttackComponent == nullptr)
    {
        return 0.0f;
    }

    return AttackComponent->GetAcquisitionRadius();*/
    return UARTOrderHelper::GetOrderRequiredRange(Order.OrderType, Owner, Order.OrderTags, Order.Index);
}

void UARTAutoOrderComponent::OnAutoOrderAdded(const FARTOrderTypeWithIndex& AutoOrder)
{
    if(Orders.Contains(AutoOrder)) return;

    int32 Index = Orders.Num();
    for(int32 i = 0; i < Orders.Num(); i ++)
    {
        if(AutoOrder.Index <= Orders[i].Index)
        {
            Index = i;
            break;
        }
    }
    
    Orders.Insert(AutoOrder, Index);

    AActor* Owner = GetOwner();

    HumanPlayerAutoOrders.InsertDefaulted(Index, 1);

    // Only certain abilities are auto cast abilities for human players.
    HumanPlayerAutoOrders[Index] = UARTOrderHelper::IsHumanPlayerAutoOrder(Orders[Index].OrderType, Owner, Orders[Index].OrderTags, Orders[Index].Index);


    // Calculating states and AI data is done on server-side, only.
    if (!Owner->HasAuthority())
    {
        return;
    }

    HumanPlayerAutoOrderStates.InsertDefaulted(Index, 1);
    AIPlayerAutoOrders.InsertDefaulted(Index, 1);

    bool bHasAutoCastOrders = false;

    if (HumanPlayerAutoOrders[Index])
    {
        bool bInitialEnabled =
            UARTOrderHelper::GetHumanPlayerAutoOrderInitialState(Orders[Index].OrderType, Owner, Orders[Index].OrderTags,Orders[Index].Index);
        bHasAutoCastOrders |= bInitialEnabled;
        HumanPlayerAutoOrderStates[Index] = bInitialEnabled;
    }

    AIPlayerAutoOrders[Index] = UARTOrderHelper::IsAIPlayerAutoOrder(Orders[Index].OrderType, Owner, Orders[Index].OrderTags,Orders[Index].Index);
        bHasAutoCastOrders |= AIPlayerAutoOrders[Index];

    if(UARTOrderComponent* OrderComponent = GetOwner()->FindComponentByClass<UARTOrderComponent>())
    {
        FARTOrderData CurrentOrder = OrderComponent->GetCurrentOrderData();
        if (bHasAutoCastOrders && UARTOrderHelper::AreAutoOrdersAllowedDuringOrder(CurrentOrder.OrderType, GetOwner(), CurrentOrder.OrderTags, CurrentOrder.Index))
        {
            bCheckAutoOrders = true;
        }
    }
}

void UARTAutoOrderComponent::OnAutoOrderRemoved(const FARTOrderTypeWithIndex& AutoOrder)
{
    if(!Orders.Contains(AutoOrder)) return;
    int32 Index = Orders.Find(AutoOrder);
    Orders.RemoveAt(Index);
    HumanPlayerAutoOrders.RemoveAt(Index);
    AIPlayerAutoOrders.RemoveAt(Index);
    HumanPlayerAutoOrderStates.RemoveAt(Index);
}


