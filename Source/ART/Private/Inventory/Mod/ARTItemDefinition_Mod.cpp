// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Mod/ARTItemDefinition_Mod.h"
#include "Inventory/Mod/ARTItemStack_Mod.h"

void FARTModPropertyHandle::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FARTModPropertyHandle::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

UARTItemDefinition_Mod::UARTItemDefinition_Mod()
	: Super()
{
	DefaultItemStackClass = UARTItemStack_Mod::StaticClass();
}

namespace ARTModPropertyHandle_Impl
{
	static int32 LastHandleId = 0;
	static int32 GetNextQueuedHandleIdForUse() { return ++LastHandleId; }
}

FARTModPropertyHandle UARTItemDefinition_Mod::GiveModPropertyTo(UAbilitySystemComponent* ASC,
	UObject* OverrideSourceObject) const
{
	check(ASC);
 
    if (!ASC->IsOwnerActorAuthoritative())
    {
        // Must be authoritative to give or take ability sets.
        return FARTModPropertyHandle();
    }
 
    FARTModPropertyHandle OutHandle;
	OutHandle.AbilitySystemComponent = ASC;
    OutHandle.HandleId = ARTModPropertyHandle_Impl::GetNextQueuedHandleIdForUse();
    
    // Grant attribute sets
    for (int32 AttributeIndex = 0; AttributeIndex < GrantedAttributeSets.Num(); ++AttributeIndex)
    {
        const FARTModProperty_Attribute& AttributeSetToGrant = GrantedAttributeSets[AttributeIndex];
 
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
        const FARTModProperty_Ability& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
 
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
        const FARTModProperty_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
 
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

FARTModPropertyHandle UARTItemDefinition_Mod::GiveModPropertyToInterface(
	TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface, UObject* OverrideSourceObject) const
{
	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(AbilitySystemInterface.GetObject());
	return GiveModPropertyTo(ASC, OverrideSourceObject);
}

void UARTItemDefinition_Mod::TakeModProperty(FARTModPropertyHandle& ModPropertyHandle)
{
	if (!ModPropertyHandle.IsValid())
	{
		return;
	}
     
	if (!ModPropertyHandle.AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}
	
	for (const FGameplayAbilitySpecHandle& Handle : ModPropertyHandle.AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ModPropertyHandle.AbilitySystemComponent->ClearAbility(Handle);
		}
	}
 
	for (const FActiveGameplayEffectHandle& Handle : ModPropertyHandle.GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			ModPropertyHandle.AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}
 
	ModPropertyHandle.Reset();
}
