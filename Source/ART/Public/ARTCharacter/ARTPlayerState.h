// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "ARTGameplayAbilitySet.h"
#include "GameplayEffectTypes.h"
#include "Inventory/Interfaces/ARTInventoryInterface.h"
#include "ARTPlayerState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FARTOnGameplayAttributeValueChangedDelegate, FGameplayAttribute,
                                               Attribute, float, NewValue, float, OldValue);


UCLASS()
class ART_API AARTPlayerState : public APlayerState, public IAbilitySystemInterface, public IARTInventoryInterface
{
	GENERATED_BODY()

public:
	AARTPlayerState();

	/** Name of the AbilitySystem component. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static FName AbilitySystemComponentName;
	static FName InventoryComponentName;
	
	// Implement IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UARTInventoryComponent* GetInventoryComponent() const override;
	
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

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attributes")
	virtual int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "ART|ARTCharacter|Attributes")
	float GetHealth() const;

	void SetHealth(const float Health);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UARTInventoryComponent_Active* InventoryComponent;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ART|Item")
	class UInventorySet* InventorySet;
	

	FGameplayTag DeadTag;
	FGameplayTag KnockedDownTag;

	// Attribute changed delegate handles
	FDelegateHandle HealthChangedDelegateHandle;

	UPROPERTY()
	class AARTCharacterBase* PlayerPawn;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	virtual void InitializeStartInventory();
};
