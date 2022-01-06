#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARTOrderPreviewSnapMode.h"
#include "ARTOrderPreview.h"
#include "ARTOrderPreviewData.generated.h"

/** Specifies how to show previews while selecting an order target. */
USTRUCT(BlueprintType)
struct ART_API FARTOrderPreviewData
{
	GENERATED_USTRUCT_BODY()

public:
	/** Actor the preview is shown for. */
	UPROPERTY()
	AActor* Owner;

	/** Actor previewing the order. */
	UPROPERTY()
	AActor* Preview;

	/** Gets the class of actors to spawn as preview for an order. */
	TSubclassOf<class AARTOrderPreview> GetOrderPreviewClass();

	/** Gets whether and how to snap order previews. */
	EARTOrderPreviewSnapMode GetSnapMode();

private:
	/** Class of actors to spawn as preview for an order. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Order", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AARTOrderPreview> OrderPreviewClass;

	/** Whether and how to snap order previews. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Order", meta = (AllowPrivateAccess = "true"))
	EARTOrderPreviewSnapMode SnapMode;
};
