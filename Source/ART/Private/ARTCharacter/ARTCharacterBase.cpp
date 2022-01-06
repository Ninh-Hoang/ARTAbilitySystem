// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTCharacterBase.h"
#include "ARTCharacter/ARTCharacterMovementComponent.h"
#include <Components/SceneComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "Item/InventoryComponent.h"
#include <Components/CapsuleComponent.h>
#include "ART/ART.h"
#include <Kismet/KismetMathLibrary.h>
#include "Ability/ARTAbilitySystemComponent.h"
#include "ARTCharacter/ARTPlayerState.h"
#include <GameFramework/PlayerState.h>
#include "ARTCharacter/ARTCharacterAttributeSet.h"
#include "ARTCharacter/ARTPlayerController.h"
#include <DrawDebugHelpers.h>
#include <Engine/World.h>
#include <TimerManager.h>
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include <Ability/ARTAbilitySystemGlobals.h>
#include <Kismet/GameplayStatics.h>
#include "Widget/ARTDamageTextWidgetComponent.h"
#include <Ability/ARTGameplayAbility.h>
#include <ARTCharacter/Voxel/ARTSimpleInvokerComponent.h>
#include <ARTCharacter/ARTGameplayAbilitySet.h>
#include <GameplayTagResponseTable.h>

// Sets default values
AARTCharacterBase::AARTCharacterBase(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UARTCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	//VoxelInvokerComponent = CreateDefaultSubobject<UARTSimpleInvokerComponent>(TEXT("VoxelInvoker"));
	//VoxelInvokerComponent->SetupAttachment(RootComponent);

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag("Effect.RemoveOnDeath");

	SetGenericTeamId(FGenericTeamId(TeamNumber));

	//setup floating status bar
	UIFloatingStatusBarComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIFloatingStatusBarComponent"));
	UIFloatingStatusBarComponent->SetupAttachment(RootComponent);
	UIFloatingStatusBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	UIFloatingStatusBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	UIFloatingStatusBarComponent->SetDrawSize(FVector2D(500, 500));
}

FGenericTeamId AARTCharacterBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamNumber);
}

ETeamAttitude::Type AARTCharacterBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	ETeamAttitude::Type Attitude = ETeamAttitude::Neutral;
	if (const AARTCharacterBase* OtherPawn = Cast<AARTCharacterBase>(&Other))
	{
		//Create an alliance with Team with ID 10 and set all the other teams as Hostiles:
		FGenericTeamId OtherTeamID = OtherPawn->GetGenericTeamId();
		if (OtherTeamID == FGenericTeamId(TeamNumber))
		{
			Attitude = ETeamAttitude::Friendly;
		}
		else if (OtherTeamID.GetId() > 50)
		{
			Attitude = ETeamAttitude::Neutral;
		}
		else
		{
			Attitude = ETeamAttitude::Hostile;
		}
	}
	return Attitude;
}

EARTHitReactDirection AARTCharacterBase::GetHitReactDirectionVector(const FVector& ImpactPoint,
                                                                    const AActor* AttackingActor)
{
	const FVector& ActorLocation = GetActorLocation();
	FVector ImpactVector;

	if (ImpactPoint.IsZero() && AttackingActor)
	{
		ImpactVector = AttackingActor->GetActorLocation();
	}
	else
	{
		ImpactVector = ImpactPoint;
	}

	// PointPlaneDist is super cheap - 1 vector subtraction, 1 dot product.
	float DistanceToFrontBackPlane = FVector::PointPlaneDist(ImpactVector, ActorLocation, GetActorRightVector());
	float DistanceToRightLeftPlane = FVector::PointPlaneDist(ImpactVector, ActorLocation, GetActorForwardVector());


	if (FMath::Abs(DistanceToFrontBackPlane) <= FMath::Abs(DistanceToRightLeftPlane))
	{
		// Determine if Front or Back

		// Can see if it's left or right of Left/Right plane which would determine Front or Back
		if (DistanceToRightLeftPlane >= 0)
		{
			return EARTHitReactDirection::Front;
		}
		return EARTHitReactDirection::Back;
	}
	// Determine if Right or Left

	if (DistanceToFrontBackPlane >= 0)
	{
		return EARTHitReactDirection::Right;
	}
	return EARTHitReactDirection::Left;

	return EARTHitReactDirection::Front;
}

UAbilitySystemComponent* AARTCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void AARTCharacterBase::RemoveCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || !AbilitySystemComponent->
		CharacterAbilitiesGiven)
	{
		return;
	}

	// Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.SourceObject == this && AbilitySet)
		{
			for (const FARTGameplayAbilityApplicationInfo& GameplayAbility : AbilitySet->StartupGameplayAbilities)
			{
				if (GameplayAbility.GameplayAbilityClass == Spec.Ability->GetClass())
				{
					AbilitiesToRemove.Add(Spec.Handle);
				}
			}
		}
	}

	// Do in two passes so the removal happens after we have the full list
	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
		UE_LOG(LogTemp, Warning, TEXT("Remove"));
	}
	AbilitySystemComponent->CharacterAbilitiesGiven = false;
}

void AARTCharacterBase::Die()
{
	// Only runs on Server
	if (!HasAuthority())
	{
		return;
	}

	RemoveCharacterAbilities();

	if (IsValid(AbilitySystemComponent) && DeathEffect)
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f,
		                                                  AbilitySystemComponent->MakeEffectContext());

		AbilitySystemComponent->AddLooseGameplayTag(DeadTag);
	}

	OnCharacterDied.Broadcast(this);

	//TODO replace with a locally executed GameplayCue
	if (DeathSound)
	{
		//UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	SetActorEnableCollision(false);
	
	if (DeathMontage)
	{
		float DeadMontagePlayTime = PlayAnimMontage(DeathMontage);

		if (!GetWorldTimerManager().IsTimerActive(ActorHiddenTimer))
		{
			//TODO: Remove magic number
			GetWorldTimerManager().SetTimer(ActorHiddenTimer, this, &AARTCharacterBase::HideActorInGame, DeadMontagePlayTime+2.f, false);
		}
	}
	else
	{
		HideActorInGame();
	}

	if (!GetWorldTimerManager().IsTimerActive(DeadDestroyTimer))
	{
		SetActorEnableCollision(false);
		//TODO: Remove Magic number 10 seconds after dead
		GetWorldTimerManager().SetTimer(DeadDestroyTimer, this, &AARTCharacterBase::FinishDying, 10.f, false);
	}
}

void AARTCharacterBase::FinishDying()
{
	Destroy();
}


void AARTCharacterBase::HideActorInGame()
{
	SetActorHiddenInGame(true);
}

void AARTCharacterBase::AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags)
{
	DamageNumberQueue.Add(FARTDamageNumber(Damage, DamageNumberTags));

	if (!GetWorldTimerManager().IsTimerActive(DamageNumberTimer))
	{
		GetWorldTimerManager().SetTimer(DamageNumberTimer, this, &AARTCharacterBase::ShowDamageNumber, 0.1, true, 0.0f);
	}
}

int32 AARTCharacterBase::GetAbilityLevel(EARTAbilityInputID AbilityID) const
{
	return 1;
}

int32 AARTCharacterBase::GetCharacterLevel() const
{
	return 1;
}

void AARTCharacterBase::AddCharacterAbilities()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || AbilitySystemComponent->CharacterAbilitiesGiven)
	{
		return;
	}

	if (AbilitySet)
	{
		AbilitySet->GiveAbilities(AbilitySystemComponent);
	}
}

void AARTCharacterBase::InitializeAttributes()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."),
		       *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DefaultAttributes, GetCharacterLevel(), EffectContext);
	if (NewHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
			*NewHandle.Data.Get(), AbilitySystemComponent);
	}
}

void AARTCharacterBase::AddStartupEffects()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent || AbilitySystemComponent->StartupEffectsApplied)
	{
		return;
	}

	if (AbilitySet)
	{
		AbilitySet->AddStartupEffects(AbilitySystemComponent);
	}
}

void AARTCharacterBase::InitializeTagPropertyMap()
{
	TagDelegateMap.Initialize(this, AbilitySystemComponent);
}

void AARTCharacterBase::InitializeTagResponseTable()
{
	if (TagReponseTable)
	{
		TagReponseTable->RegisterResponseForEvents(AbilitySystemComponent);
	}
}

void AARTCharacterBase::ShowDamageNumber()
{
	if (DamageNumberQueue.Num() > 0 && IsValid(this))
	{
		UARTDamageTextWidgetComponent* DamageText = NewObject<UARTDamageTextWidgetComponent>(this, DamageNumberClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->SetDamageText(DamageNumberQueue[0].DamageAmount, DamageNumberQueue[0].Tags);

		if (DamageNumberQueue.Num() < 1)
		{
			GetWorldTimerManager().ClearTimer(DamageNumberTimer);
		}

		DamageNumberQueue.RemoveAt(0);
	}
}

//Mostly for AI
bool AARTCharacterBase::ActivateAbilitiesWithTags(FGameplayTagContainer AbilityTags,
                                                  bool bAllowRemoteActivation /*= true*/)
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags, bAllowRemoteActivation);
	}

	return false;
}

void AARTCharacterBase::GetActiveAbilitiesWithTags(FGameplayTagContainer AbilityTags,
                                                   TArray<UARTGameplayAbility*>& ActiveAbilities)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetActiveAbilitiesWithTags(AbilityTags, ActiveAbilities);
	}
}

// Called when the game starts or when spawned
void AARTCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	/*SpawnDefaultInventory();

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSyncCurrentWeapon();
	}*/
}

// Called to bind functionality to input
void AARTCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AARTCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AARTCharacterBase::MoveRight);

	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AARTCharacterBase::Jump);

	//PlayerInputComponent->BindAxis("LookUp", this, &AARTCharacterBase::LookUp);
	//PlayerInputComponent->BindAxis("LookRight", this, &AARTCharacterBase::LookRight);

	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AARTCharacterBase::BeginCrouch);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AARTCharacterBase::EndCrouch);

	BindASCInput();
}

//movement control
void AARTCharacterBase::MoveForward(float AxisValue)
{
	FRotator Rot = FRotator(0,GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(Rot).GetScaledAxis( EAxis::X), AxisValue);
}

void AARTCharacterBase::MoveRight(float AxisValue)
{
	FRotator Rot = FRotator(0,GetControlRotation().Yaw, 0);
	AddMovementInput(FRotationMatrix(Rot).GetScaledAxis( EAxis::Y), AxisValue);
}

void AARTCharacterBase::Jump()
{
	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;
}

//camera control
void AARTCharacterBase::LookRight(float AxisValue)
{
	/*if (ensure(AzimuthComponent)) {
		AzimuthComponent->AddLocalRotation(FRotator(0, AxisValue, 0));
	}*/
	AddControllerYawInput(AxisValue);
}

void AARTCharacterBase::LookUp(float AxisValue)
{
	/*if (ensure(AzimuthComponent)) {
		AzimuthComponent->AddLocalRotation(FRotator(0, 0, AxisValue));
	}*/
	AddControllerPitchInput(AxisValue);
}

//crouch control
void AARTCharacterBase::BeginCrouch()
{
	Crouch();
}

void AARTCharacterBase::EndCrouch()
{
	UnCrouch();
}

float AARTCharacterBase::GetAttackPower() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetAttackPower();
	}

	return 0.0f;
}

float AARTCharacterBase::GetCritRate() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetCritRate();
	}

	return 0.0f;
}

float AARTCharacterBase::GetCritMultiplier() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetCritMultiplier();
	}

	return 0.0f;
}

float AARTCharacterBase::GetReactMas() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetReactMas();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPhysBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPhysBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPhysRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPhysRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetArmor() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetArmor();
	}

	return 0.0f;
}

float AARTCharacterBase::GetHealBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetHealBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetIncomingHealBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetIncomingHealBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetVoidBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetVoidBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetVoidRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetVoidRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetHeatBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetHeatBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetHeatRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetHeatRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetColdBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetColdBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetColdRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetColdRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetElecBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetElecBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetElecRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetElecRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetWaterBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetWaterBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetWaterRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetWaterRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetEarthBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetEarthBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetEarthRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetEarthRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetAirBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetAirBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GeAirRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetAirRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetLifeBonus() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetLifeBonus();
	}

	return 0.0f;
}

float AARTCharacterBase::GetLifeRes() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetLifeRes();
	}

	return 0.0f;
}

float AARTCharacterBase::GetShield() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetShield();
	}

	return 0.0f;
}

float AARTCharacterBase::GetMaxShield() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetMaxShield();
	}

	return 0.0f;
}

float AARTCharacterBase::GetShieldRegen() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetShieldRegen();
	}

	return 0.0f;
}

float AARTCharacterBase::GetHealth() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetHealth();
	}

	return 0.0f;
}

float AARTCharacterBase::GetMaxHealth() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetMaxHealth();
	}

	return 0.0f;
}

float AARTCharacterBase::GetHealthRegen() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetHealthRegen();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthA() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthA();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthB() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthB();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthC() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthC();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthD() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthD();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthE() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthE();
	}

	return 0.0f;
}

float AARTCharacterBase::GetPartHealthF() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetPartHealthF();
	}

	return 0.0f;
}

float AARTCharacterBase::GetEnergy() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetEnergy();
	}

	return 0.0f;
}

float AARTCharacterBase::GetMaxEnergy() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetMaxEnergy();
	}

	return 0.0f;
}

float AARTCharacterBase::GetEnergyRegen() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetEnergyRegen();
	}

	return 0.0f;
}

float AARTCharacterBase::GetStamina() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetStamina();
	}

	return 0.0f;
}

float AARTCharacterBase::GetMaxStamina() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetMaxStamina();
	}

	return 0.0f;
}

float AARTCharacterBase::GetStaminaRegen() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetStaminaRegen();
	}

	return 0.0f;
}

float AARTCharacterBase::GetMoveSpeed() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetMoveSpeed();
	}

	return 0.0f;
}

float AARTCharacterBase::GetRotateRate() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetRotateRate();
	}

	return 0.0f;
}

float AARTCharacterBase::GetXPMod() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetXPMod();
	}

	return 0.0f;
}

float AARTCharacterBase::GetEnMod() const
{
	if (AttributeSetBase)
	{
		return AttributeSetBase->GetEnMod();
	}

	return 0.0f;
}

void AARTCharacterBase::SetShield(float Shield)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetShield(Shield);
	}
}

void AARTCharacterBase::SetHealth(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetHealth(Health);
	}
}

void AARTCharacterBase::SetPartHealthA(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthA(Health);
	}
}

void AARTCharacterBase::SetPartHealthB(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthB(Health);
	}
}

void AARTCharacterBase::SetPartHealthC(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthC(Health);
	}
}

void AARTCharacterBase::SetPartHealthD(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthD(Health);
	}
}

void AARTCharacterBase::SetPartHealthE(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthE(Health);
	}
}

void AARTCharacterBase::SetPartHealthF(float Health)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetPartHealthF(Health);
	}
}

void AARTCharacterBase::SetStamina(float Stamina)
{
	if (AttributeSetBase)
	{
		AttributeSetBase->SetStamina(Stamina);
	}
}

// Called every frame
void AARTCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AARTCharacterBase::BindASCInput()
{
	if (!ASCInputBound && AbilitySystemComponent && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
			                                                              FString("ConfirmTarget"),
			                                                              FString("CancelTarget"),
			                                                              FString("EARTAbilityInputID"),
			                                                              static_cast<int32>(EARTAbilityInputID::Confirm
			                                                              ), static_cast<int32>(
				                                                              EARTAbilityInputID::Cancel)));

		ASCInputBound = true;
	}
}

void AARTCharacterBase::Restart()
{
	Super::Restart();
	if (AARTPlayerController* PC = Cast<AARTPlayerController>(GetController()))
	{
		//PC->ShowIngameUI();
	}
}

bool AARTCharacterBase::IsAlive() const
{
	return GetHealth() > 0.0f;
}

/* Called every frame */
void Tick(float DeltaTime)
{
}
