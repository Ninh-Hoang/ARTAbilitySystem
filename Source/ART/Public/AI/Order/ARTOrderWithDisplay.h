// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTOrder.h"
#include "ARTOrderWithDisplay.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintType, Abstract, Blueprintable)
class ART_API UARTOrderWithDisplay : public UARTOrder
{
	GENERATED_BODY()

public:
    //~ Begin URTSOrder Interface
    virtual UTexture2D* GetNormalIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual UTexture2D* GetHoveredIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual UTexture2D* GetPressedIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual UTexture2D* GetDisabledIcon(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual FText GetName(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual FText GetDescription(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    virtual int32 GetOrderButtonIndex() const override;
    virtual bool HasFixedOrderButtonIndex() const override;
	virtual FARTOrderPreviewData GetOrderPreviewData(const AActor* OrderedActor, const FGameplayTagContainer& OrderTags, int32 Index) const override;
    //~ End URTSOrder Interface

private:
    /** Normal icon of the order. Can be shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UTexture2D* NormalIcon;

    /** Hovered icon of the order. Can be shown in the UI. */
    UPROPERTY(Category = " Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UTexture2D* HoveredIcon;

    /** Pressed icon of the order. Can be shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UTexture2D* PressedIcon;

    /** Disabled icon of the order. Can be shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    UTexture2D* DisabledIcon;

    /** Name of the order. Can be shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    FText Name;

    /** Description of the order. Can be shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    FText Description;

    /** Whether to use a fixed index of the button when shown in the UI, instead of just lining it up among others. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    bool bUseFixedOrderButtonIndex;

    /** Index of the button when shown in the UI. */
    UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
    int32 OrderButtonIndex;

	UPROPERTY(Category = "Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FARTOrderPreviewData OrderPreviewData;
};
