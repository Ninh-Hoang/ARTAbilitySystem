// Fill out your copyright notice in the Description page of Project Settings.
#include "ARTCharacter/ARTGameplayAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

void FARTAbilitySetHandle::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
    if (Handle.IsValid())
    {
        AbilitySpecHandles.Add(Handle);
    }
}
 
void FARTAbilitySetHandle::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
    if (Handle.IsValid())
    {
        GameplayEffectHandles.Add(Handle);
    }
}
 
namespace ARTAbilitySetHandle_Impl
{
    static int32 LastHandleId = 0;
    static int32 GetNextQueuedHandleIdForUse() { return ++LastHandleId; }
}
 
UARTAbilitySet::UARTAbilitySet(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}
 
FARTAbilitySetHandle UARTAbilitySet::GiveAbilitySetTo(UAbilitySystemComponent* ASC, UObject* OverrideSourceObject) const
{
    check(ASC);
 
    if (!ASC->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return FARTAbilitySetHandle();
    }
 
    FARTAbilitySetHandle OutHandle;
    OutHandle.HandleId = ARTAbilitySetHandle_Impl::GetNextQueuedHandleIdForUse();
    OutHandle.AbilitySystemComponent = ASC;
    
    // Grant attribute sets
    for (int32 AttributeIndex = 0; AttributeIndex < GrantedAttributeSets.Num(); ++AttributeIndex)
    {
        const FARTAbilitySet_Attribute& AttributeSetToGrant = GrantedAttributeSets[AttributeIndex];
 
        if (!IsValid(AttributeSetToGrant.Attribute))
        {
            UE_LOG(LogAbilitySystem, Error, TEXT("GrantedAttributeSet[%d] on ability set [%s] is not valid."), AttributeIndex, *GetNameSafe(this));
            continue;
        }

        //we do not to send this to handle, we will not remove attribute set at runtime, remove ability set will only remove ability and effect
        ASC->AddAttributeSetSubobject(NewObject<UAttributeSet>(ASC->GetAvatarActor(), AttributeSetToGrant.Attribute));
    }
 
    // Grant the gameplay abilities.
    for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
    {
        const FARTAbilitySet_Ability& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
 
        if (!IsValid(AbilityToGrant.Ability))
        {
            UE_LOG(LogAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
            continue;
        }
 
        UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();
 
        FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
        AbilitySpec.SourceObject = OverrideSourceObject;
        
        const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
        OutHandle.AddAbilitySpecHandle(AbilitySpecHandle);
    }
 
    // Grant the gameplay effects.
    for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
    {
        const FARTAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
 
        if (!IsValid(EffectToGrant.GameplayEffect))
        {
            UE_LOG(LogAbilitySystem, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
            continue;
        }
 
        const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
        const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext());
        OutHandle.AddGameplayEffectHandle(GameplayEffectHandle);
    }
 
    return OutHandle;
}
 
FARTAbilitySetHandle UARTAbilitySet::GiveAbilitySetToInterface(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UObject* OverrideSourceObject) const
{
    UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(AbilitySystemInterface.GetObject());
    return GiveAbilitySetTo(ASC, OverrideSourceObject);
}
 
void UARTAbilitySet::TakeAbilitySet(FARTAbilitySetHandle& AbilitySetHandle)
{
    if (!AbilitySetHandle.IsValid())
    {
        return;
    }
     
    if (!AbilitySetHandle.AbilitySystemComponent->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return;
    }

    
 
    for (const FGameplayAbilitySpecHandle& Handle : AbilitySetHandle.AbilitySpecHandles)
    {
        if (Handle.IsValid())
        {
            AbilitySetHandle.AbilitySystemComponent->ClearAbility(Handle);
        }
    }
 
    for (const FActiveGameplayEffectHandle& Handle : AbilitySetHandle.GameplayEffectHandles)
    {
        if (Handle.IsValid())
        {
            AbilitySetHandle.AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
        }
    }
 
    AbilitySetHandle.Reset();
}
