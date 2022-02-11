// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "ARTPlayerState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTOnGameplayAttributeValueChangedDelegate, FGameplayAttribute,
                                               Attribute, float, NewValue, float, OldValue);


UCLASS()
class ART_API AARTPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AARTPlayerState();

	// Implement IAbilitySystemInterface
	class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UARTAttributeSetBase* GetAttributeSet() const;

	class UInventoryComponent* GetInventoryComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState|UI")
	void ShowAbilityConfirmPrompt(bool bShowPrompt);

	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState|UI")
	void ShowInteractionPrompt(float InteractionDuration);

	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState|UI")
	void StartInteractionTimer(float InteractionDuration);

	// Interaction interrupted, cancel and hide HUD interact timer
	UFUNCTION(BlueprintCallable, Category = "ART|ARTPlayerState|UI")
	void StopInteractionTimer();

	/**
	* Getters for attributes from GDAttributeSetBase. Returns Current Value unless otherwise specified.
	*/
	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Ability")
	bool GetCooldownRemainingForTag(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration);

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attributes")
	virtual int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attributes")
	float GetHealth() const;

	void SetHealth(float Health);
	
protected:
	UPROPERTY()
	class UARTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UARTAttributeSetBase* AttributeSet;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ART|Item")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Item")
	class UInventorySet* InventorySet;

	FGameplayTag DeadTag;
	FGameplayTag KnockedDownTag;

	// Attribute changed delegate handles
	FDelegateHandle HealthChangedDelegateHandle;

	class AARTCharacterBase* PlayerPawn;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	virtual void InitializeStartInventory();
};
