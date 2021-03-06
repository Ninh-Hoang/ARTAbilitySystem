// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTCharacter/ARTCharacterBase.h"

#include "ARTAssetManager.h"
#include "ARTCharacter/ARTCharacterMovementComponent.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "TimerManager.h"
#include "Widget/ARTStatusTextWidgetComponent.h" 
#include "ARTCharacter/ARTGameplayAbilitySet.h"
#include "GameplayTagResponseTable.h"
#include "Ability/AttributeSet/ARTAttributeSet_Health.h"
#include "Ability/AttributeSet/ARTAttributeSet_Movement.h"

FName AARTCharacterBase::AbilitySystemComponentName(TEXT("AbilitySystemComp"));
FName AARTCharacterBase::AttributeComponentName(TEXT("Attribute"));

// Sets default values
AARTCharacterBase::AARTCharacterBase(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UARTCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SetReplicateMovement(true);

	bUseControllerRotationYaw = false;
	
	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag("Effect.RemoveOnDeath");

	SetGenericTeamId(FGenericTeamId(TeamID));

	//setup floating status bar
	UIFloatingStatusBarComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIFloatingStatusBarComponent"));
	UIFloatingStatusBarComponent->SetupAttachment(RootComponent);
	UIFloatingStatusBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	UIFloatingStatusBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	UIFloatingStatusBarComponent->SetDrawSize(FVector2D(500, 500));
}

void AARTCharacterBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	OnCharacterTeamChanged.Broadcast(TeamID, NewTeamID);
	TeamID = NewTeamID;
}

FGenericTeamId AARTCharacterBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

ETeamAttitude::Type AARTCharacterBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	ETeamAttitude::Type Attitude = ETeamAttitude::Neutral;
	if (const IGenericTeamAgentInterface* Interface = Cast<IGenericTeamAgentInterface>(&Other))
	{
		//Create an alliance with Team with ID 10 and set all the other teams as Hostiles:
		FGenericTeamId OtherTeamID = Interface->GetGenericTeamId();
		if (OtherTeamID == TeamID)
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

UAbilitySystemComponent* AARTCharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void AARTCharacterBase::RemoveCharacterAbilities()
{
	if (GetLocalRole() != ROLE_Authority || !IsValid(ASC) || !ASC->
		CharacterAbilitiesGiven)
	{
		return;
	}

	for(auto& AbilitySetHandle : AbilitySetHandles)
	{
		UARTAbilitySet::TakeAbilitySet(AbilitySetHandle);
	}
	
	ASC->CharacterAbilitiesGiven = false;
}

void AARTCharacterBase::Die()
{
	// Only runs on Server
	if (!HasAuthority())
	{
		return;
	}

	RemoveCharacterAbilities();

	if (IsValid(ASC) && DeathEffect)
	{
		ASC->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = ASC->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		ASC->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f,
		                                                  ASC->MakeEffectContext());

		ASC->AddLooseGameplayTag(DeadTag);
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

int32 AARTCharacterBase::GetCharacterLevel() const
{
	return 1;
}

void AARTCharacterBase::InitializeAbilitySet()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !ASC || ASC->CharacterAbilitiesGiven)
	{
		return;
	}
	
	for(TSoftObjectPtr<UARTAbilitySet> Set : AbilitySets)
	{
		UARTAbilitySet* LoadedSet = Set.IsValid() ? Set.Get() : Set.LoadSynchronous();
		if (LoadedSet)
		{
			AbilitySetHandles.Add(LoadedSet->GiveAbilitySetTo(ASC, this));
			ASC->CharacterAbilitiesGiven = true;
		}
	}
}

void AARTCharacterBase::InitializeAttributes()
{
	if (!ASC)
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
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(
		DefaultAttributes, GetCharacterLevel(), EffectContext);
	if (NewHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	}
}

void AARTCharacterBase::InitializeTagPropertyMap()
{
	TagDelegateMap.Initialize(this, ASC);
}

void AARTCharacterBase::InitializeTagResponseTable()
{
	if (TagReponseTable)
	{
		TagReponseTable->RegisterResponseForEvents(ASC);
	}
}

void AARTCharacterBase::ShowDamageNumber()
{
	if (DamageNumberQueue.Num() > 0 && IsValid(this))
	{
		UARTStatusTextWidgetComponent* DamageText = NewObject<UARTStatusTextWidgetComponent>(this, DamageNumberClass);
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
	if (ASC)
	{
		return ASC->TryActivateAbilitiesByTag(AbilityTags, bAllowRemoteActivation);
	}

	return false;
}

void AARTCharacterBase::GetActiveAbilitiesWithTags(FGameplayTagContainer AbilityTags,
                                                   TArray<UARTGameplayAbility*>& ActiveAbilities)
{
	if (ASC)
	{
		ASC->GetActivePrimaryAbilityInstancesWithTags(AbilityTags, ActiveAbilities);
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

	//PlayerInputComponent->BindAxis("MoveForward", this, &AARTCharacterBase::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &AARTCharacterBase::MoveRight);

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

float AARTCharacterBase::GetHealth() const

{
	if (ASC)
	{
		return ASC->GetNumericAttribute(UARTAttributeSet_Health::GetHealthAttribute());
	}

	return -1.f;
}

float AARTCharacterBase::GetMaxHealth() const
{
	if (ASC)
	{
		return ASC->GetNumericAttribute(UARTAttributeSet_Health::GetMaxHealthAttribute());
	}

	return -1.f;
}

float AARTCharacterBase::GetMoveSpeed() const
{
	if (ASC)
	{
		return ASC->GetNumericAttribute(UARTAttributeSet_Movement::GetMoveSpeedAttribute());
	}

	return -1.f;
}

float AARTCharacterBase::GetRotateRate() const
{
	if (ASC)
	{
		return ASC->GetNumericAttribute(UARTAttributeSet_Movement::GetRotateRateAttribute());
	}

	return -1.f;
}

void AARTCharacterBase::SetHealth(float Health)
{
	if (ASC)
	{
		ASC->SetNumericAttributeBase(UARTAttributeSet_Health::GetHealthAttribute(), Health);
	}
}

// Called every frame
void AARTCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AARTCharacterBase::BindASCInput()
{
	if (!ASCInputBound && ASC && IsValid(InputComponent))
	{
		//TODO: find a better way of binding input
		/*ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(,
			                                                              FString("ConfirmTarget"),
			                                                              FString("CancelTarget"),
			                                                              FString("EARTAbilityInputID"),
			                                                              static_cast<int32>(EARTAbilityInputID::Confirm
			                                                              ), static_cast<int32>(
				                                                              EARTAbilityInputID::Cancel)));
		
		ASCInputBound = true;*/
	}
}

void AARTCharacterBase::Restart()
{
	Super::Restart();
	//TODO: something about UI when restart
	/*if (AARTPlayerController* PC = Cast<AARTPlayerController>(GetController()))
	{
		//PC->ShowIngameUI();
	}*/
}

bool AARTCharacterBase::IsAlive() const
{
	return GetHealth() > 0.0f;
}

/* Called every frame */
void Tick(float DeltaTime)
{
}
