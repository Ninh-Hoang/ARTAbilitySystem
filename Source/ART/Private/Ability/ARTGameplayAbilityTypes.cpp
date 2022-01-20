#include "Ability/ARTGameplayAbilityTypes.h"
#include "Ability/ARTAbilitySystemComponent.h"
#include "ARTCharacter/ARTSurvivor.h"
#include "ARTCharacter/AI/ARTCharacterAI.h"
#include "ARTCharacter/ARTPlayerController.h"
#include "ARTCharacter/ARTPlayerState.h"
#include "ARTCharacter/ARTCharacterMovementComponent.h"

void FARTGameplayAbilityActorInfo::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor,
                                                 UAbilitySystemComponent* InAbilitySystemComponent)
{
	Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);
 
	ARTAbilitySystemComponent = Cast<UARTAbilitySystemComponent>(InAbilitySystemComponent);
     
	ARTPawn = Cast<AARTCharacterBase>(InAvatarActor);
 
	if (PlayerController.IsValid())
	{
		ARTPlayerController = Cast<AARTPlayerController>(PlayerController.Get());
		ARTPlayerState = PlayerController->GetPlayerState<AARTPlayerState>();
		ARTPlayerPawn = Cast<AARTSurvivor>(InAvatarActor);
	}
	else if (ARTPawn.IsValid())
	{
		ARTAIController = ARTPawn->GetController<AARTAIController>();
		ARTAIPawn = Cast<AARTCharacterAI>(InAvatarActor);
	}

	if(MovementComponent.IsValid())
	{
		ARTCharacterMovementComponent = Cast<UARTCharacterMovementComponent>(MovementComponent);
	}
}

void FARTGameplayAbilityActorInfo::SetAvatarActor(AActor* InAvatarActor)
{
	Super::SetAvatarActor(InAvatarActor);
}

void FARTGameplayAbilityActorInfo::ClearActorInfo()
{
	Super::ClearActorInfo();
 
	ARTPawn = nullptr;
	ARTPlayerPawn = nullptr;
	ARTAIPawn = nullptr;
	ARTPlayerState = nullptr;
	ARTPlayerController = nullptr;
	ARTAbilitySystemComponent = nullptr;
	ARTAIController = nullptr;
	ARTCharacterMovementComponent = nullptr;
}

UARTAbilitySystemComponent* FARTGameplayAbilityActorInfo::GetARTAbilitySystemComponent() const
{
	return ARTAbilitySystemComponent.IsValid() ? ARTAbilitySystemComponent.Get() : nullptr;
}

AARTCharacterBase* FARTGameplayAbilityActorInfo::GetARTPawn() const
{
	return ARTPawn.IsValid() ? ARTPawn.Get() : nullptr;
}

AARTSurvivor* FARTGameplayAbilityActorInfo::GetARTPlayerPawn() const
{
	return ARTPlayerPawn.IsValid() ? ARTPlayerPawn.Get() : nullptr;
}

AARTCharacterAI* FARTGameplayAbilityActorInfo::GetARTAIPawn() const
{
	return ARTAIPawn.IsValid() ? ARTAIPawn.Get() : nullptr;
}

AARTPlayerController* FARTGameplayAbilityActorInfo::GetARTPlayerController() const
{
	return ARTPlayerController.IsValid() ? ARTPlayerController.Get() : nullptr;
}

AARTAIController* FARTGameplayAbilityActorInfo::GetARTAIController() const
{
	return ARTAIController.IsValid() ? ARTAIController.Get() : nullptr;
}

AARTPlayerState* FARTGameplayAbilityActorInfo::GetARTPlayerState() const
{
	return ARTPlayerState.IsValid() ? ARTPlayerState.Get() : nullptr;
}

UARTCharacterMovementComponent* FARTGameplayAbilityActorInfo::GetARTCharacterMovementComponent() const
{
	return ARTCharacterMovementComponent.IsValid() ? ARTCharacterMovementComponent.Get() : nullptr;
}

class AWeapon* FARTGameplayAbilityActorInfo::GetWeapon() const
{
	return GetARTPlayerPawn() ? GetARTPlayerPawn()->GetCurrentWeapon() : nullptr;
}

//***********************
//EffectSpecContainer
//***********************

bool FARTGameplayEffectContainerSpec::HasValidEffects() const
{
	return (TargetGameplayEffectSpecs.Num() > 0 || SourceGameplayEffectSpecs.Num() > 0);
}

bool FARTGameplayEffectContainerSpec::HasValidTargets() const
{
	return TargetData.Num() > 0;
}

void FARTGameplayEffectContainerSpec::AddTargets(const TArray<FGameplayAbilityTargetDataHandle>& InTargetData,
                                                 const TArray<FHitResult>& HitResults,
                                                 const TArray<AActor*>& TargetActors)
{
	for (const FGameplayAbilityTargetDataHandle& TD : InTargetData)
	{
		TargetData.Append(TD);
	}

	for (const FHitResult& HitResult : HitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetData.Add(NewData);
	}

	if (TargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->TargetActorArray.Append(TargetActors);
		TargetData.Add(NewData);
	}
}

void FARTGameplayEffectContainerSpec::ClearTargets()
{
	TargetData.Clear();
}