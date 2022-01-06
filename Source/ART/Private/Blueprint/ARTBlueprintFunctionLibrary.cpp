// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ARTBlueprintFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"
#include "Ability/ARTGameplayEffectTypes.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include <Ability/ARTGameplayEffectUIData.h>
#include <Ability/ARTGameplayAbilityUIData.h>

#include "Ability/ARTGlobalTags.h"
#include "Blueprint/SlateBlueprintLibrary.h"


FString UARTBlueprintFunctionLibrary::GetPlayerEditorWindowRole(UWorld* World)
{
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client %d "), GPlayInEditorID - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server "));
				break;
			case NM_Standalone:
				break;
			}
		}
	}

	return Prefix;
}

UARTGameplayAbility* UARTBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromHandle(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec)
		{
			return Cast<UARTGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

UARTGameplayAbility* UARTBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromClass(
	UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InAbilityClass);
		if (AbilitySpec)
		{
			return Cast<UARTGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

bool UARTBlueprintFunctionLibrary::IsPrimaryAbilityInstanceActive(UAbilitySystemComponent* AbilitySystemComponent,
                                                                  FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (AbilitySpec)
		{
			return Cast<UARTGameplayAbility>(AbilitySpec->GetPrimaryInstance())->IsActive();
		}
	}

	return false;
}

bool UARTBlueprintFunctionLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
	return Handle.IsValid();
}

bool UARTBlueprintFunctionLibrary::DoesEffectContainerSpecHaveEffects(
	const FARTGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidEffects();
}

bool UARTBlueprintFunctionLibrary::DoesEffectContainerSpecHaveTargets(
	const FARTGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidTargets();
}

void UARTBlueprintFunctionLibrary::ClearEffectContainerSpecTargets(FARTGameplayEffectContainerSpec& ContainerSpec)
{
	ContainerSpec.ClearTargets();
}

void UARTBlueprintFunctionLibrary::AddTargetsToEffectContainerSpec(FARTGameplayEffectContainerSpec& ContainerSpec,
                                                                   const TArray<FGameplayAbilityTargetDataHandle>&
                                                                   TargetData, const TArray<FHitResult>& HitResults,
                                                                   const TArray<AActor*>& TargetActors)
{
	ContainerSpec.AddTargets(TargetData, HitResults, TargetActors);
}

TArray<FActiveGameplayEffectHandle> UARTBlueprintFunctionLibrary::ApplyExternalEffectContainerSpec(
	const FARTGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// Iterate list of gameplay effects
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		if (SpecHandle.IsValid())
		{
			// If effect is valid, iterate list of targets and apply to all
			for (TSharedPtr<FGameplayAbilityTargetData> Data : ContainerSpec.TargetData.Data)
			{
				//if instigator ASC still alive
				if (SpecHandle.Data.Get()->GetContext().GetInstigatorAbilitySystemComponent())
				{
					AllEffects.Append(Data->ApplyGameplayEffectSpec(*SpecHandle.Data.Get()));
				}
					//TODO: Optimize this loop
					//apply gameplay effect to self instead
				else
				{
					for (TWeakObjectPtr<AActor> Actor : Data->GetActors())
					{
						if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent(Actor.Get()))
						{
							AllEffects.Add(ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
						}
					}
				}
			}
		}
	}
	return AllEffects;
}

FGameplayAbilityTargetDataHandle UARTBlueprintFunctionLibrary::EffectContextGetTargetData(
	FGameplayEffectContextHandle EffectContextHandle)
{
	FARTGameplayEffectContext* ARTContext = static_cast<FARTGameplayEffectContext*>(EffectContextHandle.Get());
	if (ARTContext || ARTContext->GetTargetData())
	{
		return *ARTContext->GetTargetData();
	}

	return FGameplayAbilityTargetDataHandle();
}

void UARTBlueprintFunctionLibrary::EffectContextAddTargetData(FGameplayEffectContextHandle EffectContext,
                                                              const FGameplayAbilityTargetDataHandle& TargetData,
                                                              bool Reset)
{
	FARTGameplayEffectContext* ARTContext = static_cast<FARTGameplayEffectContext*>(EffectContext.Get());
	if (ARTContext)
	{
		ARTContext->AddTargetData(TargetData, Reset);
	}
}

float UARTBlueprintFunctionLibrary::EffectContextGetKnockBackStrength(FGameplayEffectContextHandle EffectContext)
{
	FARTGameplayEffectContext* ARTContext = static_cast<FARTGameplayEffectContext*>(EffectContext.Get());
	if (ARTContext)
	{
		return ARTContext->GetKnockBackStrength();
	}
	return 0.0f;
}

void UARTBlueprintFunctionLibrary::EffectContextSetKnockBackStrength(FGameplayEffectContextHandle EffectContext,
                                                                     float InKnockBackStrength)
{
	FARTGameplayEffectContext* ARTContext = static_cast<FARTGameplayEffectContext*>(EffectContext.Get());
	if (ARTContext)
	{
		ARTContext->SetKnockBackStrength(InKnockBackStrength);
	}
}

float UARTBlueprintFunctionLibrary::EffectContextGetSourceLevel(FGameplayEffectContextHandle EffectContext)
{
	FARTGameplayEffectContext* ARTContext = static_cast<FARTGameplayEffectContext*>(EffectContext.Get());
	if (ARTContext)
	{
		return ARTContext->GetSourceLevel();
	}
	return 0.0f;
}

void UARTBlueprintFunctionLibrary::ClearTargetData(FGameplayAbilityTargetDataHandle& TargetData)
{
	TargetData.Clear();
}

FGameplayTargetDataFilterHandle UARTBlueprintFunctionLibrary::MakeTargetDataFilterByActorType(AActor* FilterActor,
	AActor* InSourceActor, TEnumAsByte<EARTTargetSelectionFilter::Type> InTargetTypeFilter,
	TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter, TSubclassOf<AActor> InRequiredActorClass,
	bool InReverseFilter)
{
	FARTTargetFilter Filter;
	Filter.SourceActor = InSourceActor;
	Filter.ActorTypeFilter = InTargetTypeFilter;
	Filter.SelfFilter = InSelfFilter;
	Filter.RequiredActorClass = InRequiredActorClass;
	Filter.bReverseFilter = InReverseFilter;

	FGameplayTargetDataFilter* NewFilter = new FARTTargetFilter(Filter);
	NewFilter->InitializeFilterContext(FilterActor);

	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}

FGameplayTargetDataFilterHandle UARTBlueprintFunctionLibrary::MakeTargetDataFilterByTeamAttitude(AActor* FilterActor,
	FGameplayTagContainer InFilterTagContainer,
	bool InFilterTag,
	TEnumAsByte<ETeamAttitude::Type> InTeamAttitude,
	TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter,
	TSubclassOf<AActor> InRequiredActorClass, bool InReverseFilter)
{
	FARTTargetFilterTeamID Filter;
	Filter.FilterTagContainer = InFilterTagContainer;
	Filter.FilterTag = InFilterTag;
	Filter.TeamAttitude = InTeamAttitude;
	Filter.SelfFilter = InSelfFilter;
	Filter.RequiredActorClass = InRequiredActorClass;
	Filter.bReverseFilter = InReverseFilter;

	FGameplayTargetDataFilter* NewFilter = new FARTTargetFilterTeamID(Filter);
	NewFilter->InitializeFilterContext(FilterActor);

	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}

TArray<FGameplayAbilityTargetDataHandle> UARTBlueprintFunctionLibrary::FilterTargetDataArray(
	TArray<FGameplayAbilityTargetDataHandle> TargetDataArray, FGameplayTargetDataFilterHandle Filterhandle)
{
	TArray<FGameplayAbilityTargetDataHandle> OutTargetDataArray;

	for (int i = 0; i < TargetDataArray.Num(); i++)
	{
		OutTargetDataArray.Add(UAbilitySystemBlueprintLibrary::FilterTargetData(TargetDataArray[i], Filterhandle));
	}

	return OutTargetDataArray;
}

FGameplayAbilityTargetDataHandle UARTBlueprintFunctionLibrary::MakeTargetDataFromHitArray(
	TArray<FHitResult>& HitResults)
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (FHitResult& Hit : HitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
		ReturnDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));
	}
	return ReturnDataHandle;
}

TArray<FGameplayAbilityTargetDataHandle> UARTBlueprintFunctionLibrary::MakeArrayTargetDataFromHitArray(
	TArray<FHitResult>& HitResults)
{
	TArray<FGameplayAbilityTargetDataHandle> ReturnDataHandles;

	for (FHitResult& Hit : HitResults)
	{
		ReturnDataHandles.Add(AbilityTargetDataFromHitResult(Hit));
	}
	return ReturnDataHandles;
}

void UARTBlueprintFunctionLibrary::InitializePropertyMap(FGameplayTagBlueprintPropertyMap& InMap, UObject* Owner,
                                                         UAbilitySystemComponent* ASC)
{
}

float UARTBlueprintFunctionLibrary::GetTagCallerMag(UAbilitySystemComponent* InASC,
                                                    FActiveGameplayEffectHandle& InActiveHandle, FGameplayTag CallerTag)
{
	if (InASC && InActiveHandle.IsValid())
	{
		const FActiveGameplayEffect* ActiveGE = InASC->GetActiveGameplayEffect(InActiveHandle);
		return ActiveGE->Spec.GetSetByCallerMagnitude(CallerTag);
	}
	return 0.0f;
}

FGameplayTagContainer UARTBlueprintFunctionLibrary::GetAssetTagFromSpec(FGameplayEffectSpecHandle SpecHandle)
{
	return SpecHandle.Data->Def->InheritableGameplayEffectTags.CombinedTags;
}

UARTGameplayEffectUIData* UARTBlueprintFunctionLibrary::GetGameplayEffectUIDataFromActiveHandle(
	const FActiveGameplayEffectHandle& InActiveHandle)
{
	if (InActiveHandle.IsValid())
	{
		if (const UAbilitySystemComponent* ASC = InActiveHandle.GetOwningAbilitySystemComponent())
		{
			const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(InActiveHandle);
			UGameplayEffectUIData* Data = ActiveEffect->Spec.Def->UIData;
			return Cast<UARTGameplayEffectUIData>(Data);
		}
	}
	return nullptr;
}

UARTGameplayAbilityUIData* UARTBlueprintFunctionLibrary::GetGameplayAbilityUIDataFromInput(
	UAbilitySystemComponent* InASC, const EARTAbilityInputID Input)
{
	if (InASC)
	{
		if (FGameplayAbilitySpec* Spec = InASC->FindAbilitySpecFromInputID(static_cast<int32>(Input)))
		{
			UARTGameplayAbility* Ability = Cast<UARTGameplayAbility>(Spec->Ability);
			//return Cast<UARTGameplayAbilityUIData>(Ability->UIData);
		}
	}
	return nullptr;
}

bool UARTBlueprintFunctionLibrary::ProjectWorldToScreenBidirectional(APlayerController* Player,
                                                                     const FVector& WorldPosition,
                                                                     FVector2D& ScreenPosition,
                                                                     bool& bTargetBehindCamera,
                                                                     bool bPlayerViewportRelative)
{
	FVector Projected;
	bool bSuccess = false;

	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		// get the projection data
		FSceneViewProjectionData ProjectionData;
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData))
		{
			const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();

			FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));

			if (Result.W < 0.f) { bTargetBehindCamera = true; }
			else { bTargetBehindCamera = false; }

			if (Result.W == 0.f) { Result.W = 1.f; } // Prevent Divide By Zero

			const float RHW = 1.f / FMath::Abs(Result.W);
			Projected = FVector(Result.X, Result.Y, Result.Z) * RHW;

			// Normalize to 0..1 UI Space
			const float NormX = (Projected.X / 2.f) + 0.5f;
			const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

			Projected.X = (float)ViewRectangle.Min.X + (NormX * (float)ViewRectangle.Width());
			Projected.Y = (float)ViewRectangle.Min.Y + (NormY * (float)ViewRectangle.Height());

			bSuccess = true;
			ScreenPosition = FVector2D(Projected.X, Projected.Y);

			if (bPlayerViewportRelative)
			{
				ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
			}
		}
		else
		{
			ScreenPosition = FVector2D(1234, 5678);
		}
	}

	return bSuccess;
}

bool UARTBlueprintFunctionLibrary::ProjectWorldToWidgetBidirectional(APlayerController* Player,
                                                                     const FVector& WorldPosition,
                                                                     FVector2D& ViewportPosition,
                                                                     bool& bTargetBehindCamera,
                                                                     bool bPlayerViewportRelative)
{
	if (Player)
	{
		FVector2D ScreenPosition2D;
		const bool bProjected = ProjectWorldToScreenBidirectional(Player, WorldPosition, ScreenPosition2D,
		                                                          bTargetBehindCamera, bPlayerViewportRelative);

		if (bProjected)
		{
			FVector2D ViewportPosition2D;
			USlateBlueprintLibrary::ScreenToViewport(Player, ScreenPosition2D, ViewportPosition2D);
			ViewportPosition.X = ViewportPosition2D.X;
			ViewportPosition.Y = ViewportPosition2D.Y;
			return true;
		}
	}

	ViewportPosition = FVector2D::ZeroVector;

	return false;
}

bool UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirements(const FGameplayTagContainer& Tags,
                                                              const FGameplayTagContainer& RequiredTags,
                                                              const FGameplayTagContainer& BlockedTags)
{
	if (BlockedTags.Num() || RequiredTags.Num())
	{
		if (Tags.HasAny(BlockedTags))
		{
			return false;
		}

		if (!Tags.HasAll(RequiredTags))
		{
			return false;
		}
	}

	return true;
}

bool UARTBlueprintFunctionLibrary::DoesSatisfyTagRequirementsWithResult(const FGameplayTagContainer& Tags,
                                                                        const FGameplayTagContainer& InRequiredTags,
                                                                        const FGameplayTagContainer& InBlockedTags,
                                                                        FGameplayTagContainer& OutMissingTags,
                                                                        FGameplayTagContainer& OutBlockingTags)
{
	bool bSuccess = true;
	for (FGameplayTag Tag : InBlockedTags)
	{
		if (Tags.HasTag(Tag))
		{
			bSuccess = false;
			OutBlockingTags.AddTag(Tag);
		}
	}

	for (FGameplayTag Tag : InRequiredTags)
	{
		if (!Tags.HasTag(Tag))
		{
			bSuccess = false;
			OutMissingTags.AddTag(Tag);
		}
	}

	return bSuccess;
}

void UARTBlueprintFunctionLibrary::GetTags(const AActor* Actor, FGameplayTagContainer& OutGameplayTags)
{
	OutGameplayTags = OutGameplayTags.EmptyContainer;
	if (!Actor)
	{
		return;
	}

	const UAbilitySystemComponent* AbilitySystem = Actor->FindComponentByClass<UAbilitySystemComponent>();
	if (AbilitySystem == nullptr)
	{
		return;
	}

	AbilitySystem->GetOwnedGameplayTags(OutGameplayTags);
}

void UARTBlueprintFunctionLibrary::GetSourceAndTargetTags(const AActor* SourceActor, const AActor* TargetActor,
                                                          FGameplayTagContainer& OutSourceTags,
                                                          FGameplayTagContainer& OutTargetTags)
{
	GetTags(SourceActor, OutSourceTags);
	GetTags(TargetActor, OutTargetTags);

	FGameplayTagContainer RelationshipTags = GetTeamAttitudeTags(SourceActor, TargetActor);

	OutSourceTags.AppendTags(RelationshipTags);
	OutTargetTags.AppendTags(RelationshipTags);
}

FGameplayTagContainer UARTBlueprintFunctionLibrary::GetTeamAttitudeTags(const AActor* Actor, const AActor* Other)
{
	FGameplayTagContainer RelationshipTags;
	if (Actor == nullptr || Other == nullptr)
	{
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Neutral());
		return RelationshipTags;
	}
	if (Actor == Other)
	{
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Friendly());
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Self());
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Visible());
		return RelationshipTags;
	}
	
	const AARTCharacterBase* SourceCharacter = Cast<AARTCharacterBase>(Actor);
	ETeamAttitude::Type TeamAttitude = SourceCharacter->GetTeamAttitudeTowards(*Other);

	switch (TeamAttitude)
	{
	case ETeamAttitude::Friendly:
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Friendly());
		break;
	case ETeamAttitude::Neutral:
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Neutral());
		break;
	case ETeamAttitude::Hostile:
		RelationshipTags.AddTag(UARTGlobalTags::Behaviour_Hostile());
		break;
	default:
		break;
	}
	return RelationshipTags;
}

bool UARTBlueprintFunctionLibrary::IsVisibleForActor(const AActor* Actor, const AActor* Other)
{
	if (Actor == nullptr || Other == nullptr)
	{
		return false;
	}

	//TODO: Implement stealth..etc here
	return true;
}

FVector UARTBlueprintFunctionLibrary::GetGroundLocation2D(AARTAIController* Controller, const FVector2D Location2D)
{
	FVector ViewLocation;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRot);

	return FVector(0);
}
