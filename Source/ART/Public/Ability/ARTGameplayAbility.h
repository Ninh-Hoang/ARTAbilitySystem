// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Ability/ARTGameplayAbilityTypes.h"
#include "AI/Order/ARTOrderGroupExecutionType.h"
#include "AI/Order/ARTOrderPreviewData.h"
#include "AI/Order/ARTOrderTagRequirements.h"
#include "AI/Order/ARTOrderTargetData.h"
#include "AI/Order/ARTOrderTargetType.h"
#include "ART/ART.h"
#include "Blueprint/ARTCurve.h"
#include "ARTGameplayAbility.generated.h"

/**
 * 
 */
class USkeletalMeshComponent;

USTRUCT()
struct FAbilityMeshMontage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class USkeletalMeshComponent* Mesh;

	UPROPERTY()
	class UAnimMontage* Montage;

	FAbilityMeshMontage() : Mesh(nullptr), Montage(nullptr)
	{
	}

	FAbilityMeshMontage(class USkeletalMeshComponent* InMesh, class UAnimMontage* InMontage)
		: Mesh(InMesh), Montage(InMontage)
	{
	}
};

/**
* Describes how an ability is executed. This might determine how the ability is displayed in the UI and it determines
* how the ability is handled by the order system.
* Note that this has nothing to do with the effects an ability might apply to a target. This has also nothing todo with
* the cooldown of the ability.
*/
UENUM(BlueprintType)
enum class EAbilityProcessPolicy : uint8
{
	/**
	* The ability has no duration. If it has been activated it is considered to be finished instantly. Note that this
	* does not necessarily mean that 'EndAbility' has already been called. It only means that the order system will not
	* wait until this ability is really finished before it proceeds and it will not actively cancel the ability if
	* another order is issued.
	*/
	INSTANT UMETA(DisplayName = "Instant"),

	/**
	* The ability is considered to have a duration. The order system will wait until the ability has been finished
	* ('EndAbility has been called') but will actively cancel the ability when another order is issued.
	*/
	CAN_BE_CANCELED UMETA(DisplayName = "Can be Canceled"),

	/**
	* The ability is considered to have a duration. The order system will wait until the ability has been finished
	* ('EndAbility has been called') regardless whether another order has been issued.
	*/
	CAN_NOT_BE_CANCELED UMETA(DisplayName = "Can not be Canceled"),

	// clang-format off

	/**
	* Same as 'CanBeCanceled' with the only difference that the ability can not be canceled as long an an ability task
	* is active (e.g. 'PlayMontageAndWaitWithNotify').
	*/
	CAN_BE_CANCELED_WHEN_NO_GAMEPLAY_TASK_IS_RUNNING UMETA(
		DisplayName = "Can be Canceled when no Gameplay Task is running")
	// clang-format on
};

USTRUCT(BlueprintType)
struct FAbilityFloat{
	
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Value;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag CurveTag;
};

UCLASS()
class ART_API UARTGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UARTGameplayAbility();

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability|Activation")
	EARTAbilityInputID AbilityID = EARTAbilityInputID::None;
	
	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability|Activation")
	EARTAbilityInputID AbilityInputID = EARTAbilityInputID::None;

	// Tells an ability to activate immediately when its granted. Used for passive abilities and abilities forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bActivateAbilityOnGranted;

	// allow remote activated for forcing ability
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bAllowRemoteGrantingActivation;

	// If true, this ability will activate when its bound input is pressed. Disable if you want to bind an ability to an
	// input but not have it activate when pressed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bActivateOnInput;

	// If true, only activate this ability if the weapon that granted it is the currently equipped weapon.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bSourceObjectMustEqualCurrentWeaponToActivate;

	// If true, only activate this ability if not interacting with something via GA_Interact.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bCannotActivateWhileInteracting;

	// If true, ability will be canceled when leveled up
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Activation")
	bool bCancelWhenLevelup;

	//Ability that can stack/charge or not, 1 mean no stack
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ability|Data")
	TSoftObjectPtr<UARTCurve> AbilityData;
	
	//Ability that can stack/charge or not, 1 mean no stack
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Ability|Data")
	int32 AbilityCharge = 1;

	//cooldown duration
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability|Data")
	FAbilityFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability|Data")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability|Data")
	FAbilityFloat Cost;

	/** Ability will be cancel if these tag added on ASC */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer AbilityCancelTag;

	// Map of gameplay tags to gameplay effect containers
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffects")
	TMap<FGameplayTag, FARTGameplayEffectContainer> EffectContainerMap;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	//Do not called this directly, this is only used for logic after ability is activated
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	// Make gameplay effect container spec to be applied later, using the passed in container
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual FARTGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(
		const FARTGameplayEffectContainer& Container, const FGameplayEventData& EventData,
		int32 OverrideGameplayLevel = -1);

	// Search for and make a gameplay effect container spec to be applied later, from the EffectContainerMap
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual FARTGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag,
	                                                                const FGameplayEventData& EventData,
	                                                                int32 OverrideGameplayLevel = -1);

	// Applies a gameplay effect container spec that was previously created
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(
		const FARTGameplayEffectContainerSpec& ContainerSpec);

	// Expose GetSourceObject to Blueprint. Retrieves the SourceObject associated with this ability. Callable on non instanced abilities.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability", meta = (DisplayName = "Get Source Object"))
	UObject* K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	// Attempts to activate the given ability handle and batch all RPCs into one. This will only batch all RPCs that happen
	// in one frame. Best case scenario we batch ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	// Same as calling K2_EndAbility. Meant for use with batching system to end the ability externally.
	virtual void ExternalEndAbility();

	// Returns the current prediction key and if it's valid for more predicting. Used for debugging ability prediction windows.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	// Returns if the current prediction key is valid for more predicting.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Override cooldown related function for dynamic cooldown GE
	// ----------------------------------------------------------------------------------------------------------------

	/** Event for generating utility score */

	bool bHasBlueprintScoreUtility;
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName="ScoreAbilityutility",
		meta=(ScriptName="ScoreAbilityUtility"))
	float K2_ScoreAbilityUtility() const;
	UFUNCTION(BlueprintPure, Category=Order)
	float ScoreAbilityUtility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr,
	                                OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                       OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           OUT FGameplayTagContainer* OptionalRelevantTags /* = nullptr */) const override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void OnCooldownTagEventCallback(const FGameplayTag CallbackTag, int32 NewCount);

	virtual void OnCancelTagEventCallback(const FGameplayTag CallbackTag, int32 NewCount);

	// Allows C++ and Blueprint abilities to override how cost is checked in case they don't use a GE like weapon ammo
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool ARTCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool ARTCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	                                         const FGameplayAbilityActorInfo& ActorInfo) const;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                       const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	int32 GetCurrentCharge();

	// Allows C++ and Blueprint abilities to override how cost is applied in case they don't use a GE like weapon ammo
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void ARTApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo,
	                  const FGameplayAbilityActivationInfo ActivationInfo) const;

	virtual void ARTApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	                                         const FGameplayAbilityActorInfo& ActorInfo,
	                                         const FGameplayAbilityActivationInfo ActivationInfo) const
	{
	};

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SetHUDReticle(TSubclassOf<class UARTHUDReticle> ReticleClass);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void ResetHUDReticle();

	// Sends TargetData from the client to the Server and creates a new Prediction Window
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData);

	// Is the player's input currently pressed? Only works if the ability is bound to input.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool IsInputPressed() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Animation Support for multiple USkeletalMeshComponents on the AvatarActor
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns the currently playing montage for this ability, if any */
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	/** Call to set/get the current montage from a montage task. Set to allow hooking up montage events to ability events */
	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, class UAnimMontage* InCurrentMontage);

	/** Applies a gameplay effect container, by creating and then applying the spec */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(FGameplayTag ContainerTag,
	                                                                 const FGameplayEventData& EventData,
	                                                                 int32 OverrideGameplayLevel = -1);

	//TODO: FIX this node
	UFUNCTION(BlueprintCallable, Category = "Ability|GameplayEffect")
	void BP_ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec& AbilitySpec) const;

protected:
	int32 CurrentCharges = 0;

	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;

	//Array of handles for CancelTag
	TMap<FGameplayTag, FDelegateHandle> RegisteredCancelTagEventHandles;

	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY()
	FGameplayTagContainer TempCooldownTags;

	// ----------------------------------------------------------------------------------------------------------------
	//	Animation Support for multiple USkeletalMeshComponents on the AvatarActor
	// ----------------------------------------------------------------------------------------------------------------

	/** Active montages being played by this ability */
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;

	bool FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMontage);

	/** Immediately jumps the active montage to a section */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);

	/** Sets pending section on active montage */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);

	/**
	 * Stops the current animation montage.
	 * @param InMesh mesh with montage that need to be stop
	 * @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	/**
	* Stops all currently animating montages
	*
	* @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForAllMeshes(float OverrideBlendOutTime = -1.0f);

public:
	// ----------------------------------------------------------------------------------------------------------------
	//	ARTAvatarActorInfo Getter
	// ----------------------------------------------------------------------------------------------------------------

	/*const FARTGameplayAbilityActorInfo* GetARTActorInfo(const FGameplayAbilityActorInfo* Info) const;

	UFUNCTION(BlueprintPure, Category = "Ability|ActorInfo")
	FARTGameplayAbilityActorInfo BP_GetARTActorInfo();

	UFUNCTION(BlueprintPure, Category = "Ability|ActorInfo")
	UAnimInstance* GetAnimInstance() const;

	UFUNCTION(BlueprintPure, Category = "Ability|ActorInfo")
	AWeapon* BP_GetWeapon() const;*/


protected:
	/**
	* Describes how this ability is executed. This might determine how the ability is displayed in the UI and it
	* determines how the ability is handled by the order system. Note that this has nothing todo with the effects an
	* ability might apply to a target. This has also nothing todo with the cooldown of the ability.
	*/

	UPROPERTY(Category = "Ability|Order", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EAbilityProcessPolicy AbilityProcessPolicy;

	/**
	 * To how many and which of the selected units should this order be issued to.
	 */
	UPROPERTY(Category = "Ability|Order", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTOrderGroupExecutionType GroupExecutionType;

	/**
	 * The target type of this ability.
	 */

	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FScalableFloat AbilityBaseRange;

	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EARTTargetType TargetType;

	/** Details about the preview for this ability while choosing a target. */
	UPROPERTY(Category = "Ability|Targeting", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FARTOrderPreviewData AbilityPreviewData;

	/** Icon of the ability. Can be shown in the UI. */
	UPROPERTY(Category = "Ability|Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UTexture2D* Icon;

	/** Name of the ability. Can be shown in the UI. */
	UPROPERTY(Category = "Ability|Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FText Name;

	/** Description of the ability. Can be shown in the UI. */
	UPROPERTY(Category = "Ability|Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FText Description;

	/** Whether to show this ability as a default order in the UI, instead of as an ability. */
	UPROPERTY(Category = "Ability|Display", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bShouldShowAsOrderInUI;

	/** The specific acquisition radius for this ability. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly,
		meta = (AllowPrivateAccess = true, EditCondition = bIsAcquisitionRadiusOverridden))
	float AcquisitionRadiusOverride;

	/** Whether this ability uses a specific acquisition radius. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsAcquisitionRadiusOverridden; // InlineEditConditionToggle caused Editor crashes here.

	/** Auto order priority, lower means will be prioritized */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly,meta = (AllowPrivateAccess = true))
	int32 AutoOrderPriority;
	
	/** Whether this ability is an auto ability for the human player. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bHumanPlayerAutoAbility;

	/** When this ability is an auto ability, this value indicates whether it is active by default. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = true))
	bool bHumanPlayerAutoAutoAbilityInitialState;

	/** Whether this ability is an auto ability for AI players. */
	UPROPERTY(Category = "Ability|AutoOrder", BlueprintReadOnly, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	bool bAIPlayerAutoAbility;

	/** Whether this ability uses a specific target score. */
	UPROPERTY(Category = "Ability|AutoOrder", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsTargetScoreOverridden;

public:
	/*
	 *Order related function
	 */
	/** Gets the target type of this ability. */
	EARTTargetType GetTargetType() const;

	/** Gets the group execution type this ability. */
	EARTOrderGroupExecutionType GetGroupExecutionType() const;

	/**
	 * Gets the event trigger tag of this ability is triggered with. Note that this just returns the first event tag
	 * found in AbilityTriggers.
	 */
	UFUNCTION(Category = Order, BlueprintPure)
	FGameplayTag GetEventTriggerTag() const;

	/** Gets the AbilityTriggerData of this ability. */
	const TArray<FAbilityTriggerData>& GetAbilityTriggerData() const;

	/** Gets the icon of this ability. Can be shown in the UI. */
	UFUNCTION(Category = Order, BlueprintPure)
	UTexture2D* GetIcon() const;

	/** Gets the name of this ability. Can be shown in the UI. */
	UFUNCTION(Category = Order, BlueprintPure)
	FText GetName() const;

	/** Gets the description of this ability. Can be shown in the UI. */
	UFUNCTION(Category = Order, BlueprintPure)
	FText GetDescription(const AActor* Actor) const;

	/** Gets details about the preview for this ability while choosing a target. */
	FARTOrderPreviewData GetAbilityPreviewData() const;

	/** Formats the description by replacing any placeholders by actual values. */
	UFUNCTION(Category = Order, BlueprintNativeEvent)
	void FormatDescription(const FText& InDescription, const AActor* Actor, FText& OutDescription) const;
	virtual void FormatDescription_Implementation(const FText& InDescription, const AActor* Actor,
	                                              FText& OutDescription) const;

	/**
	 * Called by the owning gameplay ability system when the level of the owner has been changed. Note that this is not
	 * called on non instanced abilities.
	 */
	UFUNCTION(Category = Order, BlueprintNativeEvent)
	void OnAbilityLevelChanged(int32 NewLevel);
	virtual void OnAbilityLevelChanged_Implementation(int32 NewLevel);

	/**
	 * Evaluates the target and returns a score that can be used to compare the different targets. A higher score means
	 * a better target for the ability.
	 */
	bool bHasBlueprintGetTargetScore;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityTargetScore",
		meta=(ScriptName="AbilityTargetScore"))
	float K2_GetTargetScore(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData, int32 Index) const;


	float GetTargetScore(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, const FARTOrderTargetData& TargetData, int32 Index) const;

	bool bHasBlueprintGetOrderTargetData;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityOrderTargetData",
		meta=(ScriptName="AbilityOrderTargetData"))
	bool K2_GetOrderTargetData(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle, FARTOrderTargetData& OrderTargetData) const;
	
	bool GetOrderTargetData(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle, OUT FARTOrderTargetData* OrderTargetData = nullptr) const;
	
	/** Gets the ability process policy of this ability. */
	EAbilityProcessPolicy GetAbilityProcessPolicy() const;

	/** Gets the tags the activating actor needs to have in order to activate the ability. */
	UFUNCTION(Category = Order, BlueprintPure)
	FGameplayTagContainer GetActivationRequiredTags() const;

	/** Gets order tag requirements that corresponds with the tags of this ability. */
	void GetOrderTagRequirements(FARTOrderTagRequirements& OutTagRequirements) const;

	/** Gets source tag requirements of this ability. */
	void GetSourceTagRequirements(FGameplayTagContainer& OutRequiredTags, FGameplayTagContainer& OutBlockedTags) const;

	/** Gets target tag requirements of this ability. */
	void GetTargetTagRequirements(FGameplayTagContainer& OutRequiredTags, FGameplayTagContainer& OutBlockedTags) const;

	/**
	 * Gets the minimum range between the caster and the target that is needed to activate the ability.
	 * '0' value is returned if the ability has no range.
	 */

	bool bHasBlueprintGetRange;
	
	UFUNCTION(BlueprintImplementableEvent, Category = Order, DisplayName="AbilityRange",
		meta=(ScriptName="AbilityRange"))
	float K2_GetRange(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle) const;

	float GetRange(FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpecHandle Handle) const;
	
	/** Gets the specific acquisition radius for this ability. */
	bool GetAcquisitionRadiusOverride(float& OutAcquisitionRadius) const;

	int32 GetAutoOrderPriority() const;

	/** Whether this ability is an auto ability for the human player. */
	bool IsHumanPlayerAutoAbility() const;

	/** When this ability is an auto ability, this value indicates whether it is active by default. */
	bool GetHumanPlayerAutoAutoAbilityInitialState() const;

	/** Whether this ability is an auto ability for AI players. */
	bool IsAIPlayerAutoAbility() const;

	/** Whether this ability uses a specific target score. */
	bool IsTargetScoreOverriden() const;

	/** Whether there are ability tasks active on this gameplay ability instance. */
	bool AreAbilityTasksActive() const;

	//~ Begin UGameplayAbility Interface
	virtual bool ShouldActivateAbility(ENetRole Role) const override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	//~ End UGameplayAbility Interface

	UFUNCTION(BlueprintPure, Category="Order")
	FVector GetBlackboardOrderLocation();

	UFUNCTION(BlueprintPure, Category="Order")
	AActor* GetBlackboardOrderTarget();

	UFUNCTION(BlueprintPure, Category="Order")
	FVector GetBlackboardOrderHomeLocation();

	UFUNCTION(BlueprintPure, Category="Order")
	float GetBlackboardOrderRange();

	UFUNCTION(BlueprintPure, Category="Ability|Targeting")
	bool DoesSatisfyTargetTagRequirement(AActor* Target);
};
