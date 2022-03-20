// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ARTBlueprintFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/ARTGameplayAbility.h"
#include "Ability/ARTGameplayEffectTypes.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/ARTGameplayEffectUIData.h"
#include "Ability/ARTGlobalTags.h"
#include "ARTCharacter/AI/ARTAIController.h"
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

void UARTBlueprintFunctionLibrary::AddTargetToEffectContainerSpec(FARTGameplayEffectContainerSpec& ContainerSpec,
	const FGameplayAbilityTargetDataHandle& TargetData, const FHitResult& HitResult, AActor* TargetActor)
{
	ContainerSpec.AddTarget(TargetData, HitResult, TargetActor);
}

TArray<FActiveGameplayEffectHandle> UARTBlueprintFunctionLibrary::ApplyExternalEffectContainerSpec(
	const FARTGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;
	
	// Iterate list of gameplay effects
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
			// If effect is valid, iterate list of targets and apply to all
			if(!SpecHandle.IsValid()) continue;
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

	for(const FGameplayEffectSpecHandle& SpecHandle :ContainerSpec.SourceGameplayEffectSpecs)
	{
		// If effect is valid, iterate list of targets and apply to all
		if(!SpecHandle.IsValid()) continue;
			//if instigator ASC still alive
		if (UAbilitySystemComponent* ASC = SpecHandle.Data.Get()->GetContext().GetInstigatorAbilitySystemComponent())
		{
			AllEffects.Add(ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
		}
	}
	
	return AllEffects;
}

FGameplayEffectSpecHandle UARTBlueprintFunctionLibrary::AssignTagMagnitudeInEffectContainerSpec(
	const FARTGameplayEffectContainerSpec& ContainerSpec, FGameplayTag DataTag, float Magnitude, bool IsTargetSpec,
	int32 Index)
{
	//check if index is valid
	if(IsTargetSpec && Index > (ContainerSpec.TargetGameplayEffectSpecs.Num() -1)) return FGameplayEffectSpecHandle();
	if(!IsTargetSpec && Index > (ContainerSpec.SourceGameplayEffectSpecs.Num() -1)) return FGameplayEffectSpecHandle();
	
	const FGameplayEffectSpecHandle* SpecHandle;
	if(IsTargetSpec) SpecHandle = &ContainerSpec.TargetGameplayEffectSpecs[Index];
	else SpecHandle = &ContainerSpec.SourceGameplayEffectSpecs[Index];

	FGameplayEffectSpec* Spec = SpecHandle->Data.Get();
	
	if (Spec)
	{
		Spec->SetSetByCallerMagnitude(DataTag, Magnitude);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilitySystemBlueprintLibrary::AssignSetByCallerTagMagnitude called with invalid SpecHandle"));
	}

	return *SpecHandle;
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

UGameplayAbility* UARTBlueprintFunctionLibrary::EffectContextGetAbility(FGameplayEffectContextHandle EffectContext)
{
	//do not edit this, just read
	return const_cast<UGameplayAbility*>(EffectContext.GetAbility());
}

void UARTBlueprintFunctionLibrary::GetAbilityTags(const UGameplayAbility* Ability, FGameplayTagContainer& AbilityTags)
{
	if(!Ability) return;
	AbilityTags.Reset();
	AbilityTags = Ability->AbilityTags;
}

void UARTBlueprintFunctionLibrary::ClearTargetData(FGameplayAbilityTargetDataHandle& TargetData)
{
	TargetData.Clear();
}

FGameplayAbilityTargetDataHandle UARTBlueprintFunctionLibrary::FilterTargetDataByActorType(
    const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	AActor* InContextActor, TEnumAsByte<EARTTargetSelectionFilter::Type> InTargetTypeFilter,
	TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter, TSubclassOf<AActor> InRequiredActorClass,
	bool InReverseFilter)
{
	return FilterTargetData(TargetDataHandle, MakeTargetDataFilterByActorType(InContextActor, InTargetTypeFilter, InSelfFilter, InRequiredActorClass, InReverseFilter));
}

FGameplayTargetDataFilterHandle UARTBlueprintFunctionLibrary::MakeTargetDataFilterByActorType(AActor* InContextActor,
	TEnumAsByte<EARTTargetSelectionFilter::Type> InTargetTypeFilter,
	TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter, TSubclassOf<AActor> InRequiredActorClass,
	bool InReverseFilter)
{
	FARTTargetDataFilter_TargetType Filter;
	Filter.ActorTypeFilter = InTargetTypeFilter;
	Filter.SelfFilter = InSelfFilter;
	Filter.RequiredActorClass = InRequiredActorClass;
	Filter.bReverseFilter = InReverseFilter;

	FGameplayTargetDataFilter* NewFilter = new FARTTargetDataFilter_TargetType(Filter);
	NewFilter->InitializeFilterContext(InContextActor);

	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}

FGameplayAbilityTargetDataHandle UARTBlueprintFunctionLibrary::FilterTargetDataByTeamAttitude(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	AActor* InContextActor,
	const FGameplayTagContainer& RequiredTags,
	const FGameplayTagContainer& BlockedTags,
	const FGameplayTagContainer& BehaviourTags,
	TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter,
	TSubclassOf<AActor> InRequiredActorClass,
	bool InReverseFilter)
{
	return FilterTargetData(TargetDataHandle, MakeTargetDataFilterByTeamAttitude(InContextActor, RequiredTags, BlockedTags, BehaviourTags, InSelfFilter, InRequiredActorClass, InReverseFilter));
}

FGameplayTargetDataFilterHandle UARTBlueprintFunctionLibrary::MakeTargetDataFilterByTeamAttitude(
	AActor* InContextActor, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& BlockedTags,
	const FGameplayTagContainer& BehaviourTags, TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter,
	TSubclassOf<AActor> InRequiredActorClass, bool InReverseFilter)
{
	FARTTargetDataFilter_TeamAttitude Filter;
	Filter.RequiredTags = RequiredTags;
	Filter.BlockedTags = BlockedTags;
	Filter.BehaviourTags = BehaviourTags;
	Filter.SelfFilter = InSelfFilter;
	Filter.RequiredActorClass = InRequiredActorClass;
	Filter.bReverseFilter = InReverseFilter;

	FGameplayTargetDataFilter* NewFilter = new FARTTargetDataFilter_TeamAttitude(Filter);
	NewFilter->InitializeFilterContext(InContextActor);

	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}

TArray<FGameplayAbilityTargetDataHandle> UARTBlueprintFunctionLibrary::FilterTargetDataArray(
	TArray<FGameplayAbilityTargetDataHandle> TargetDataArray, FGameplayTargetDataFilterHandle FilterHandle)
{
	TArray<FGameplayAbilityTargetDataHandle> OutTargetDataArray;

	for (int i = 0; i < TargetDataArray.Num(); i++)
	{
		OutTargetDataArray.Add(FilterTargetData(TargetDataArray[i], FilterHandle));
	}

	return OutTargetDataArray;
}

FGameplayAbilityTargetDataHandle UARTBlueprintFunctionLibrary::MakeTargetDataFromHitArray(
	TArray<FHitResult>& HitResults)
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (const FHitResult& Hit : HitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
		ReturnDataHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));
	}
	return ReturnDataHandle;
}

void UARTBlueprintFunctionLibrary::MakeArrayTargetDataFromHitArray(const TArray<FHitResult>& HitResults, TArray<FGameplayAbilityTargetDataHandle>& TargetData)
{
	TargetData.Empty();
	for (const FHitResult& Hit : HitResults)
	{
		TargetData.Add(AbilityTargetDataFromHitResult(Hit));
	}
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

FGameplayTagContainer& UARTBlueprintFunctionLibrary::GetAssetTagFromSpec(const FGameplayEffectSpecHandle& SpecHandle)
{
	/*if(!SpecHandle.IsValid()) return;
	Container.Reset();
	SpecHandle.Data->GetAllAssetTags(Container);*/
	
	if(!SpecHandle.IsValid()) return *new FGameplayTagContainer();
	FGameplayTagContainer Container;
	SpecHandle.Data->GetAllAssetTags(Container);
	FGameplayTagContainer* ContainerPointer = new FGameplayTagContainer(Container);
	return *ContainerPointer;
	
}

FGameplayTagContainer& UARTBlueprintFunctionLibrary::GetGrantedTagFromSpec(const FGameplayEffectSpecHandle& SpecHandle)
{
	if(!SpecHandle.IsValid()) return *new FGameplayTagContainer();
	FGameplayTagContainer Container;
	SpecHandle.Data->GetAllGrantedTags(Container);
	FGameplayTagContainer* ContainerPointer = new FGameplayTagContainer(Container);
	return *ContainerPointer;
}

float UARTBlueprintFunctionLibrary::GetDuration(const FGameplayEffectSpecHandle& SpecHandle)
{
	if(!SpecHandle.IsValid()) return 0.f;
	float Duration = 0.f;
	SpecHandle.Data->AttemptCalculateDurationFromDef(Duration);
	return Duration;
}

FGameplayEffectSpecHandle UARTBlueprintFunctionLibrary::AddDuration(FGameplayEffectSpecHandle SpecHandle,
	float AddDuration)
{
	if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
	{
		const float NewDuration = GetDuration(Spec) + AddDuration > 0.01f;
		if (NewDuration > 0.01f)
		{
			Spec->SetDuration(NewDuration, false);
		}
		else
		{
			Spec->SetDuration(0.01f, false);
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UARTAbilitySystemBlueprintLibrary::AddDuration called with invalid SpecHandle"));
	}

	return SpecHandle;
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
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, /*eSSP_FULL*/  /*out*/ ProjectionData))
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
			OutBlockingTags.AddTagFast(Tag);
		}
	}

	for (FGameplayTag Tag : InRequiredTags)
	{
		if (!Tags.HasTag(Tag))
		{
			bSuccess = false;
			OutMissingTags.AddTagFast(Tag);
		}
	}

	return bSuccess;
}

FGameplayTagQuery UARTBlueprintFunctionLibrary::MakeGameplayTagQuery_AnyTag(const FGameplayTagContainer& AnyTags)
{
	return FGameplayTagQuery::MakeQuery_MatchAnyTags(AnyTags);
}

FGameplayTagQuery UARTBlueprintFunctionLibrary::MakeQuery_MatchAllTags(FGameplayTagContainer const& AllTags)
{
	return FGameplayTagQuery::MakeQuery_MatchAllTags(AllTags);
}

FGameplayTagQuery UARTBlueprintFunctionLibrary::MakeQuery_MatchNoTags(FGameplayTagContainer const& NoneOfTags)
{
	return FGameplayTagQuery::MakeQuery_MatchNoTags(NoneOfTags);
}

void UARTBlueprintFunctionLibrary::GetTags(const AActor* Actor, FGameplayTagContainer& OutGameplayTags)
{
	OutGameplayTags = OutGameplayTags.EmptyContainer;
	if (!Actor)
	{
		return;
	}

	const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Actor);
	
	if (!ASCInterface) return;
	
	const UAbilitySystemComponent* AbilitySystem = ASCInterface->GetAbilitySystemComponent();
	
	if (!AbilitySystem) return;

	AbilitySystem->GetOwnedGameplayTags(OutGameplayTags);
}

void UARTBlueprintFunctionLibrary::GetSourceAndTargetTags(const AActor* SourceActor, const AActor* TargetActor,
                                                          FGameplayTagContainer& OutSourceTags,
                                                          FGameplayTagContainer& OutTargetTags)
{
	GetTags(SourceActor, OutSourceTags);
	GetTags(TargetActor, OutTargetTags);

	FGameplayTagContainer RelationshipTags;
	GetTeamAttitudeTags(SourceActor, TargetActor, RelationshipTags);

	OutSourceTags.AppendTags(RelationshipTags);
	OutTargetTags.AppendTags(RelationshipTags);
}

void UARTBlueprintFunctionLibrary::GetTeamAttitudeTags(const AActor* Actor, const AActor* Other, FGameplayTagContainer& AttitudeTags)
{
	AttitudeTags.Reset();
	
	if(!Actor || !Other) return;
	
	if (Actor == Other)
	{
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Friendly);
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Self);
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Visible);
		return;
	}

	const IGenericTeamAgentInterface* SourceCharacter = Cast<IGenericTeamAgentInterface>(Actor);
	
	const ETeamAttitude::Type TeamAttitude = SourceCharacter->GetTeamAttitudeTowards(*Other);

	switch (TeamAttitude)
	{
	case ETeamAttitude::Friendly:
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Friendly);
		break;
	case ETeamAttitude::Neutral:
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Neutral);
		break;
	case ETeamAttitude::Hostile:
		AttitudeTags.AddTagFast(FARTGlobalTags::Get().Behaviour_Hostile);
		break;
	default:
		break;
	}
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

FVector UARTBlueprintFunctionLibrary::GetGroundLocation2D(const AActor* WorldObject, const FVector2D Location2D)
{
	FVector StartLocation(Location2D.X, Location2D.Y, 10000);
	FVector EndLocation(Location2D.X, Location2D.Y, -10000);
	
	FCollisionQueryParams TraceParams;
	FCollisionObjectQueryParams ObjectParams(ECollisionChannel::ECC_WorldStatic);

	TraceParams.bTraceComplex = false;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bReturnFaceIndex = false;


	//Re-initialize hit info
	FHitResult OutHit(ForceInit);

	//call GetWorld() from within an actor extending class
	if(WorldObject->GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECollisionChannel::ECC_WorldStatic, TraceParams))
	{
		return OutHit.Location;
	}

	
	return FVector(0);
}
