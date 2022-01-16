// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "VoxelCharacter.h"
#include "AbilitySystemInterface.h"
#include "ART/ART.h"
#include "GameplayEffectTypes.h"
#include <GenericTeamAgentInterface.h>

#include "ARTAttributeSetBase.h"

#include "ARTCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterDiedDelegate, AARTCharacterBase*, Character);

class AWeapon;

USTRUCT(BlueprintType)
struct ART_API FARTDamageNumber
{
	GENERATED_USTRUCT_BODY()

	float DamageAmount;

	FGameplayTagContainer Tags;

	FARTDamageNumber()
	{
	}

	FARTDamageNumber(float InDamageAmount, FGameplayTagContainer InTags) : DamageAmount(InDamageAmount)
	{
		// Copy tag container
		Tags.AppendTags(InTags);
	}
};

UCLASS()
class ART_API AARTCharacterBase : public ACharacter, public IAbilitySystemInterface,
                                  public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AARTCharacterBase(const class FObjectInitializer& ObjectInitializer);

	static FName AbilitySystemComponentName;
	static FName AttributeComponentName;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//class UARTSimpleInvokerComponent* VoxelInvokerComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Character")
	FText CharacterName;

	//Team ID, handled by Interface, different ID are hostile except +50 ID as Neutral
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ART|Character")
	int TeamNumber;

	FGenericTeamId GetGenericTeamId() const override;

	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	//hit direction
	UFUNCTION(BlueprintCallable, Category = "ART|Character")
	EARTHitReactDirection GetHitReactDirectionVector(const FVector& ImpactPoint, const AActor* AttackingActor);

	// Implement IAbilitySystemInterface
	class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void RemoveCharacterAbilities();

	//public loose tag action
	//DIE stuffs

	UPROPERTY(BlueprintAssignable, Category = "ART|Character")
	FCharacterDiedDelegate OnCharacterDied;

	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "ART|Character")
	virtual void FinishDying();

	virtual void AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags);

protected:
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;
	
	FTimerHandle ActorHiddenTimer;
	FTimerHandle DeadDestroyTimer;

	TArray<FARTDamageNumber> DamageNumberQueue;
	FTimerHandle DamageNumberTimer;

	//Tag and FProperty respond Mapper
	UPROPERTY(EditAnywhere, Category = "ART|Delegate")
	FGameplayTagBlueprintPropertyMap TagDelegateMap;

	//Tag Response Table
	UPROPERTY(EditAnywhere, Category = "ART|Delegate")
	class UGameplayTagReponseTable* TagReponseTable;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "ART|Character")
	TSubclassOf<class UGameplayEffect> DeathEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Animation")
	UAnimMontage* Knockback;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Audio")
	class USoundCue* DeathSound;

	//ABILITY SYSTEM STUFFS
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTAbilitySystemComponent* ASC;

	UPROPERTY()
	class UARTAttributeSetBase* Attribute;

	
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Abilities")
	class UARTGameplayAbilitySet* AbilitySet;
	
	// Default attributes for a character for initializing on spawn/respawn.
	// This is an instant GE that overrides the values for attributes that get reset on spawn/respawn.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ART|UI")
	TSubclassOf<class UARTFloatingStatusBarWidget> UIFloatingStatusBarClass;

	UPROPERTY()
	class UARTFloatingStatusBarWidget* UIFloatingStatusBar;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "ART|UI")
	class UWidgetComponent* UIFloatingStatusBarComponent;
	
	UPROPERTY(EditAnywhere, Category = "ART|UI")
	TSubclassOf<class UARTStatusTextWidgetComponent> DamageNumberClass;

	// Grant abilities on the Server. The Ability Specs will be replicated to the owning client.
	virtual void AddCharacterAbilities();

	// Initialize the Character's attributes. Must run on Server but we run it on Client too
	// so that we don't have to wait. The Server's replication to the Client won't matter since
	// the values should be the same.
	virtual void InitializeAttributes();

	virtual void AddStartupEffects();

	virtual void InitializeTagPropertyMap();

	virtual void InitializeTagResponseTable();

	virtual void ShowDamageNumber();

	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//movement
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	virtual void Jump() override;

	//camera control
	void LookRight(float AxisValue);
	void LookUp(float AxisValue);

	//crouch
	void BeginCrouch();
	void EndCrouch();


	//bind ASC input
	bool ASCInputBound = false;
	void BindASCInput();

	virtual void Restart() override;

	void HideActorInGame();

public:
	//FOR AI/Manual Blueprint Ability Activation
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    bool ActivateAbilitiesWithTags(FGameplayTagContainer AbilityTags, bool bAllowRemoteActivation = true);
       
    /** Returns a list of active abilities matching the specified tags. This only returns if the ability is currently running */
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    void GetActiveAbilitiesWithTags(FGameplayTagContainer AbilityTags, TArray<class UARTGameplayAbility*>& ActiveAbilities);

	// Switch on AbilityID to return individual ability levels. Hardcoded to 1 for every ability in this project.
	UFUNCTION(BlueprintCallable, Category = "ART|Character")
	virtual int32 GetAbilityLevel(EARTAbilityInputID AbilityID) const;

	UFUNCTION(BlueprintCallable, Category = "ART|Character")
	virtual int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attribute")
	bool IsAlive() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attribute")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attribute")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attribute")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attribute")
	float GetRotateRate() const;
	

	//ONLY USE THIS FOR RE/SPAWNING
	void SetHealth(float Health);
};
