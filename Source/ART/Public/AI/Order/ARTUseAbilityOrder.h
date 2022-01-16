// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTOrderWithBehavior.h"

#include "ARTOrderErrorTags.h"
#include "ARTOrderTargetType.h"

#include "ARTUseAbilityOrder.generated.h"

/**
* Orders the actor to use a ability.
*/
class AActor;
struct FARTOrderTargetData;
class UARTAbilitySystemComponent;
class UGameplayAbility;

UCLASS(NotBlueprintType, Abstract, Blueprintable, hideCategories = ("Display", "Requirements", "Policy"))
class ART_API UARTUseAbilityOrder : public UARTOrderWithBehavior
{
	GENERATED_BODY()

public:
	UARTUseAbilityOrder();
	
	//~ Begin UARTOrder Interface
	virtual bool CanObeyOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
	                          FARTOrderErrorTags* OutErrorTags = nullptr) const override;


	virtual EARTTargetType GetTargetType(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                     int32 Index) const override;
	virtual void IssueOrder(AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	                        const FGameplayTagContainer& OrderTags, int32 Index,
	                        FARTOrderCallback Callback, const FVector& HomeLocation) override;
	virtual UTexture2D* GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                  int32 Index) const override;
	virtual UTexture2D* GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                   int32 Index) const override;
	virtual UTexture2D* GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                   int32 Index) const override;
	virtual UTexture2D* GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                    int32 Index) const override;
	virtual FText GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                      int32 Index) const override;
	virtual FText GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                             int32 Index) const override;
	virtual void GetTagRequirements(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index,
	                                FARTOrderTagRequirements& OutTagRequirements) const override;

	virtual float GetRequiredRange(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                               int32 Index) const override;
	virtual FARTOrderPreviewData GetOrderPreviewData(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                                 int32 Index) const override;
	virtual EARTOrderProcessPolicy GetOrderProcessPolicy(const AActor* OrderedActor,
	                                                     const FGameplayTagContainer& OrderTags,
	                                                     int32 Index) const override;
	virtual EARTOrderGroupExecutionType GetGroupExecutionType(const AActor* OrderedActor,
	                                                          const FGameplayTagContainer& OrderTags,
	                                                          int32 Index) const override;
	virtual bool IsHumanPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                    int32 Index) const override;
	virtual bool GetHumanPlayerAutoOrderInitialState(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                                 int32 Index) const override;
	virtual bool IsAIPlayerAutoOrder(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                 int32 Index) const override;
	virtual bool GetAcquisitionRadiusOverride(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags,
	                                          int32 Index,
	                                          float& OutAcquisitionRadius) const override;
	virtual float GetTargetScore(const AActor* OrderedActor, const FARTOrderTargetData& TargetData,
	                             const FGameplayTagContainer& OrderTags, int32 Index) const;
	virtual void InitializePreviewActor(AARTOrderPreview* PreviewActor, const AActor* OrderedActor,
	                                    const FARTOrderTargetData& TargetData,
	                                    const FGameplayTagContainer& OrderTags, int32 Index) override;
	//~ End UARTOrder Interface
protected:
	virtual UGameplayAbility* GetAbility(const UARTAbilitySystemComponent* AbilitySystem, int32 Index) const;
	virtual UGameplayAbility* GetAbility(const UARTAbilitySystemComponent* AbilitySystem,
	                                     FGameplayTagContainer OrderTags) const;
private:
	UTexture2D* GetIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const;
};
