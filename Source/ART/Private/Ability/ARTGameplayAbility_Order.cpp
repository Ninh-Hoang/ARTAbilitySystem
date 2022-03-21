// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTGameplayAbility_Order.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

UARTGameplayAbility_Order::UARTGameplayAbility_Order()
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& (Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass()));
	};
	{
		static FName FuncName = FName(TEXT("K2_GetTargetScore"));
		UFunction* TargetScoreFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetTargetScore = ImplementedInBlueprint(TargetScoreFunction);
	}
	
	{
		static FName FuncName = FName(TEXT("K2_GetOrderTargetData"));
		UFunction* OrderTargetDataFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetOrderTargetData = ImplementedInBlueprint(OrderTargetDataFunction);
	}
	
	{
		static FName FuncName = FName(TEXT("K2_GetRange"));
		UFunction* AbilityRangeFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintGetRange = ImplementedInBlueprint(AbilityRangeFunction);
	}
	
	//order system
	AbilityProcessPolicy = EAbilityProcessPolicy::INSTANT;

	GroupExecutionType = EARTOrderGroupExecutionType::MOST_SUITABLE_UNIT;

	AcquisitionRadiusOverride = 0.0f;
	bIsAcquisitionRadiusOverridden = false;

	bHumanPlayerAutoAbility = false;
	bHumanPlayerAutoAutoAbilityInitialState = false;
	bAIPlayerAutoAbility = false;
}

EARTTargetType UARTGameplayAbility_Order::GetTargetType() const
{
	return TargetType;
}

EAbilityProcessPolicy UARTGameplayAbility_Order::GetAbilityProcessPolicy() const
{
	return AbilityProcessPolicy;
}

EARTOrderGroupExecutionType UARTGameplayAbility_Order::GetGroupExecutionType() const
{
	return GroupExecutionType;
}

FARTOrderPreviewData UARTGameplayAbility_Order::GetAbilityPreviewData() const
{
	return AbilityPreviewData;
}

float UARTGameplayAbility_Order::GetRange(FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpecHandle Handle) const
{
	if(bHasBlueprintGetRange)
	{
		return K2_GetRange(*ActorInfo, Handle);
	}
	return AbilityBaseRange.GetValue();
}

float UARTGameplayAbility_Order::GetTargetScore(FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData, int32 Index) const
{
	if(bHasBlueprintGetTargetScore)
	{
		return K2_GetTargetScore(*ActorInfo, Handle, TargetData, Index);
	}
	return 0.0f;
}

bool UARTGameplayAbility_Order::GetOrderTargetData(FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpecHandle Handle, FARTOrderTargetData* OrderTargetData) const
{
	if(bHasBlueprintGetOrderTargetData)
	{
		return K2_GetOrderTargetData(*ActorInfo, Handle, *OrderTargetData);
	}
	return false;
}

void UARTGameplayAbility_Order::GetOrderTagRequirements(FARTOrderTagRequirements& OutTagRequirements) const
{
	OutTagRequirements.SourceRequiredTags.AppendTags(SourceRequiredTags);
	OutTagRequirements.SourceBlockedTags.AppendTags(SourceBlockedTags);
	OutTagRequirements.TargetRequiredTags.AppendTags(TargetRequiredTags);
	OutTagRequirements.TargetBlockedTags.AppendTags(TargetBlockedTags);
}

bool UARTGameplayAbility_Order::GetAcquisitionRadiusOverride(float& OutAcquisitionRadius) const
{
	OutAcquisitionRadius = AcquisitionRadiusOverride;
	return bIsAcquisitionRadiusOverridden;
}

int32 UARTGameplayAbility_Order::GetAutoOrderPriority() const
{
	return AutoOrderPriority;
}

bool UARTGameplayAbility_Order::IsHumanPlayerAutoAbility() const
{
	return bHumanPlayerAutoAbility;
}

bool UARTGameplayAbility_Order::GetHumanPlayerAutoAutoAbilityInitialState() const
{
	return bHumanPlayerAutoAutoAbilityInitialState;
}

bool UARTGameplayAbility_Order::IsAIPlayerAutoAbility() const
{
	return bAIPlayerAutoAbility;
}

bool UARTGameplayAbility_Order::IsTargetScoreOverriden() const
{
	return bIsTargetScoreOverridden;
}

FVector UARTGameplayAbility_Order::GetBlackboardOrderLocation() const
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsVector(FName("Order_Location"));
}

AActor* UARTGameplayAbility_Order::GetBlackboardOrderTarget() const
{
	return Cast<AActor>(UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsObject(FName("Order_Target")));
}

FVector UARTGameplayAbility_Order::GetBlackboardOrderHomeLocation() const
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsVector(FName("Order_HomeLocation"));
}

float UARTGameplayAbility_Order::GetBlackboardOrderRange() const
{
	return UAIBlueprintHelperLibrary::GetBlackboard(GetAvatarActorFromActorInfo())->GetValueAsFloat(FName("Order_Range"));
}
