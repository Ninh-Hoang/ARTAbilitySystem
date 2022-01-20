// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "ARTTargetFilter.h"
#include "ARTBlueprintFunctionLibrary.generated.h"

/**
 * 
 */

class UARTGameplayEffectUIData;
class UARTGameplayAbilityUIData;

UCLASS()
class ART_API UARTBlueprintFunctionLibrary : public UAbilitySystemBlueprintLibrary
{
	GENERATED_BODY()

public:
	//returns the player's editor window role - Server (listen host) or Client #

	UFUNCTION(BlueprintPure)
	static FString GetPlayerEditorWindowRole(UWorld* World);

	/**
	 * Gameplay Ability Stuffs
	 */

	UFUNCTION(BlueprintPure, Category = "Ability")
	static UARTGameplayAbility* GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent,
	                                                                FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Ability")
	static UARTGameplayAbility* GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent,
	                                                               TSubclassOf<UGameplayAbility> InAbilityClass);

	UFUNCTION(BlueprintPure, Category = "Ability")
	static bool IsPrimaryAbilityInstanceActive(UAbilitySystemComponent* AbilitySystemComponent,
	                                           FGameplayAbilitySpecHandle Handle);


	/*
	*FGameplayEffectSpecHandle
	*/
	UFUNCTION(BlueprintPure, BlueprintPure, Category = "GameplayEffectSpec")
	static bool IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle);
	
	/*
	*GameplayEffectContainerSpec
	*/

	// USTRUCTs cannot contain UFUNCTIONS so we make static functions here
	// Checks if spec has any effects
	UFUNCTION(BlueprintPure, Category = "Ability|Container")
	static bool DoesEffectContainerSpecHaveEffects(const FARTGameplayEffectContainerSpec& ContainerSpec);

	// Checks if spec has any targets
	UFUNCTION(BlueprintPure, Category = "Ability|Container")
	static bool DoesEffectContainerSpecHaveTargets(const FARTGameplayEffectContainerSpec& ContainerSpec);

	// Clears spec's targets
	UFUNCTION(BlueprintCallable, Category = "Ability|Container")
	static void ClearEffectContainerSpecTargets(UPARAM(ref) FARTGameplayEffectContainerSpec& ContainerSpec);

	// Adds targets to a copy of the passed in effect container spec and returns it
	UFUNCTION(BlueprintCallable, Category = "Ability|Container", Meta = (AutoCreateRefTerm =
		"TargetData, HitResults, TargetActors"))
	static void AddTargetsToEffectContainerSpec(UPARAM(ref) FARTGameplayEffectContainerSpec& ContainerSpec,
	                                            const TArray<FGameplayAbilityTargetDataHandle>& TargetData,
	                                            const TArray<FHitResult>& HitResults,
	                                            const TArray<AActor*>& TargetActors);

	// Applies container spec that was made from an ability
	UFUNCTION(BlueprintCallable, Category = "Ability|Container")
	static TArray<FActiveGameplayEffectHandle> ApplyExternalEffectContainerSpec(
		const FARTGameplayEffectContainerSpec& ContainerSpec);

	/**
	* FARTGameplayEffectContext
	*/

	// Returns TargetData
	UFUNCTION(BlueprintPure, Category = "Ability|EffectContext", Meta = (DisplayName = "GetTargetData"))
	static FGameplayAbilityTargetDataHandle EffectContextGetTargetData(FGameplayEffectContextHandle EffectContext);

	// Adds TargetData
	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", Meta = (DisplayName = "AddTargetData"))
	static void EffectContextAddTargetData(FGameplayEffectContextHandle EffectContext,
	                                       const FGameplayAbilityTargetDataHandle& TargetData, bool Reset);

	// Returns KnockBackStrength
	UFUNCTION(BlueprintPure, Category = "Ability|EffectContext", Meta = (DisplayName = "GetKnockBackStrength"))
    static float EffectContextGetKnockBackStrength(FGameplayEffectContextHandle EffectContext);

	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", Meta = (DisplayName = "SetKnockBackStrength"))
    static void EffectContextSetKnockBackStrength(FGameplayEffectContextHandle EffectContext, float InKnockBackStrength);
	
	// Returns SourceLevel
	UFUNCTION(BlueprintPure, Category = "Ability|EffectContext", Meta = (DisplayName = "GetSourceLevel"))
    static float EffectContextGetSourceLevel(FGameplayEffectContextHandle EffectContext);
	
	/**
	* FGameplayAbilityTargetDataHandle
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|TargetData")
	static void ClearTargetData(UPARAM(ref) FGameplayAbilityTargetDataHandle& TargetData);

	/*
	* FARTTargetFilterHandle filter by type
	*/
	UFUNCTION(BlueprintPure, Category = "Ability|TargetData", Meta = (DisplayName =
		"Make Target Data Filter by Target Type"))
	static FGameplayTargetDataFilterHandle MakeTargetDataFilterByActorType(AActor* FilterActor, AActor* InSourceActor,
	                                                                       TEnumAsByte<EARTTargetSelectionFilter::Type>
	                                                                       InTargetTypeFilter,
	                                                                       TEnumAsByte<ETargetDataFilterSelf::Type>
	                                                                       InSelfFilter,
	                                                                       TSubclassOf<AActor> InRequiredActorClass,
	                                                                       bool InReverseFilter);

	/*
	* FARTTargetFilterHandle filter by TeamAttitude
	*/
	UFUNCTION(BlueprintPure, Category = "Ability|TargetData", Meta = (DisplayName =
		"Make Target Data Filter by Team Attitude"))
	static FGameplayTargetDataFilterHandle MakeTargetDataFilterByTeamAttitude(
		AActor* FilterActor,
		FGameplayTagContainer InFilterTagContainer,
		TEnumAsByte<ETeamAttitude::Type> InTeamAttitude,
		TEnumAsByte<ETargetDataFilterSelf::Type> InSelfFilter, TSubclassOf<AActor> InRequiredActorClass,
		bool InReverseFilter);
	/*
	* Filter Bulk TargetData
	*/
	UFUNCTION(BlueprintPure, Category = "Ability|TargetData", Meta = (DisplayName = "Filter Target Data Array"))
	static TArray<FGameplayAbilityTargetDataHandle> FilterTargetDataArray(
		TArray<FGameplayAbilityTargetDataHandle> TargetDataArray,
		FGameplayTargetDataFilterHandle Filterhandle);

	/*
	* turn HitResult in to TargetData
	*/
	UFUNCTION(BlueprintPure, Category = "Ability|TargetData", Meta = (DisplayName = "Ability Target Data from Hit Result Array"))
	static FGameplayAbilityTargetDataHandle MakeTargetDataFromHitArray(TArray<FHitResult>& HitResults);

	UFUNCTION(BlueprintPure, Category = "Ability|TargetData",  Meta = (DisplayName = "Ability Target Data Array from Hit Result Array"))
	static TArray<FGameplayAbilityTargetDataHandle> MakeArrayTargetDataFromHitArray(TArray<FHitResult>& HitResults);

	/*
	* GameplayTagBlueprintPropertyMap ultilities
	*/
	static void InitializePropertyMap(FGameplayTagBlueprintPropertyMap& InMap, UObject* Owner,
	                                  UAbilitySystemComponent* ASC);

	/*
	* Get Tag caller Mag
	*/
	UFUNCTION(BlueprintPure, Category = "Ability|ActiveEffect")
	static float GetTagCallerMag(UAbilitySystemComponent* InASC, FActiveGameplayEffectHandle& InActiveHandle,
	                             FGameplayTag CallerTag);

	UFUNCTION(BlueprintPure, Category = "Ability|GameplayEffect")
	static FGameplayTagContainer GetAssetTagFromSpec(FGameplayEffectSpecHandle SpecHandle);
	/*
	* Get UI information from GameplayEffect Handle, or Spec?
	*/

	UFUNCTION(BlueprintPure, Category = "Ability|ActiveEffect", Meta = (DisplayName =
		"Get GE UI Data from Active Handle"))
	static UARTGameplayEffectUIData* GetGameplayEffectUIDataFromActiveHandle(
		const FActiveGameplayEffectHandle& InActiveHandle);
	
	/*
	 * GetWorldLocation to Screen/Widget space
	 */
	 
	UFUNCTION(BlueprintPure, Category = "UI Math")
	static bool ProjectWorldToScreenBidirectional(APlayerController* Player, const FVector& WorldPosition, FVector2D& ScreenPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative = false);

	UFUNCTION(BlueprintPure, Category = "UI Math")
    static bool ProjectWorldToWidgetBidirectional(APlayerController* Player, const FVector& WorldPosition, FVector2D& ViewportPosition, bool& bTargetBehindCamera, bool bPlayerViewportRelative = false);

	/*
	 * ORDER HELPER
	 * /
	/** Checks if the specified tags has all of the specified required tags and none of the specified blocked tags. */
	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static bool DoesSatisfyTagRequirements(const FGameplayTagContainer& Tags, const FGameplayTagContainer& RequiredTags,
										const FGameplayTagContainer& BlockedTags);

	/** Checks if the specified tags has all of the specified required tags and none of the specified blocked tags. */
	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static bool DoesSatisfyTagRequirementsWithResult(const FGameplayTagContainer& Tags,
													const FGameplayTagContainer& InRequiredTags,
													const FGameplayTagContainer& InBlockedTags,
													FGameplayTagContainer& OutMissingTags,
													FGameplayTagContainer& OutBlockingTags);

	/** Gets the gameplay tags of the specified actor. */
	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static void GetTags(const AActor* Actor, FGameplayTagContainer& OutGameplayTags);

	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static void GetSourceAndTargetTags(const AActor* SourceActor, const AActor* TargetActor, FGameplayTagContainer& OutSourceTags, FGameplayTagContainer& OutTargetTags);

	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static FGameplayTagContainer GetTeamAttitudeTags(const AActor* Actor, const AActor* Other);

	/** Whether 'Other' is visible for 'Actor'. */
	UFUNCTION(Category = "Order|Tags", BlueprintPure)
	static bool IsVisibleForActor(const AActor* Actor, const AActor* Other);

	UFUNCTION(Category = "Order|Data", BlueprintPure)
	static FVector GetGroundLocation2D(AARTAIController* Controller, const FVector2D Location2D);

	
};
