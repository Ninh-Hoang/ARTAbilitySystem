#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "ARTGameplayAbilityTypes.generated.h"

USTRUCT(BlueprintType)
struct FARTGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_USTRUCT_BODY()
     
        typedef Super FGameplayAbilityActorInfo;
     
	virtual ~FARTGameplayAbilityActorInfo()
	{
	}

	//Our ART AbilitySystemComponent. Should NEVER be null.
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<class UARTAbilitySystemComponent> ARTAbilitySystemComponent;

	//Our ART Pawn. Often nullptr.
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<class AARTCharacterBase> ARTPawn;
 
	//Our ART Player Pawn. Often nullptr
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo") 
	TWeakObjectPtr<class AARTSurvivor> ARTPlayerPawn;
	
	//Our ART AI Pawn. Often nullptr
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo") 
	TWeakObjectPtr<class AARTCharacterAI> ARTAIPawn;
 
	//Our ART Player Controller. Often nullptr.
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<class AARTPlayerController> ARTPlayerController;
 
	//Our ART AI Controller. Often nullptr.
 	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo") 
 	TWeakObjectPtr<class AARTAIController> ARTAIController;
 	
	//Our ART Player State. Often nullptr.
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<class AARTPlayerState> ARTPlayerState;

	//Our ART MovementComp. Often nullptr.
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo") 
	TWeakObjectPtr<class UARTCharacterMovementComponent> ARTCharacterMovementComponent;

	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
 
	virtual void SetAvatarActor(AActor* AvatarActor) override;
 
	virtual void ClearActorInfo() override;

	//Gets the ART AbilitySystemComponent. This should NEVER NULL.
	class UARTAbilitySystemComponent* GetARTAbilitySystemComponent() const;
	
	//Gets the ART Pawn. This is often nullptr.
	class AARTCharacterBase* GetARTPawn() const;
 
	//Gets the ART Player Pawn. This is often nullptr.
	class AARTSurvivor* GetARTPlayerPawn() const;

	//Gets the ART AI Pawn. This is often nullptr.
	class AARTCharacterAI* GetARTAIPawn() const;
 
	//Gets the ART Player Controller. This is often nullptr.
	class AARTPlayerController* GetARTPlayerController() const;

	//Gets the ART Player Controller. This is often nullptr.
	class AARTAIController* GetARTAIController() const;
	
	//Gets the ART Player State. This is often nullptr.
	class AARTPlayerState* GetARTPlayerState() const;

	//Gets the ART MovementComp. This is often nullptr.
	class UARTCharacterMovementComponent* GetARTCharacterMovementComponent() const;

	class AWeapon* GetWeapon() const;
};

class UARTAbilitySystemComponent;
class UGameplayEffect;
class UARTTargetType;

/**
* 
*/

USTRUCT(BlueprintType)
struct FARTGameplayEffectContainer
{
	GENERATED_BODY()

public:
	FARTGameplayEffectContainer()
	{
	}

	/** Sets the way that targeting happens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	TSubclassOf<UARTTargetType> TargetType;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;

	/** List of gameplay effects to apply to the source */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	TArray<TSubclassOf<UGameplayEffect>> SourceGameplayEffectClasses;
};

/** A "processed" version of GSGameplayEffectContainer that can be passed around and eventually applied */
USTRUCT(BlueprintType)
struct FARTGameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	FARTGameplayEffectContainerSpec()
	{
	}

	/** Computed target data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	FGameplayAbilityTargetDataHandle TargetData;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/** List of gameplay effects to apply to the source */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer")
	TArray<FGameplayEffectSpecHandle> SourceGameplayEffectSpecs;

	/** Returns true if this has any valid effect specs */
	bool HasValidEffects() const;

	/** Returns true if this has any valid targets */
	bool HasValidTargets() const;

	/** Adds new targets to target data */
	void AddTargets(const TArray<FGameplayAbilityTargetDataHandle>& TargetData, const TArray<FHitResult>& HitResults,
                    const TArray<AActor*>& TargetActors);

	/** Clears target data */
	void ClearTargets();
};