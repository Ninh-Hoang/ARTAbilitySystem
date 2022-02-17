// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/ARTAbilityTagRelationship.h"

void UARTAbilityTagRelationship::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
	for (const FARTAbilityTagRelationshipItem& Relationship : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Relationship.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Relationship.AbilityTagsToBlock);
			}
             
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Relationship.AbilityTagsToCancel);
			}
		}
	}
}

void UARTAbilityTagRelationship::GetActivationRequiredAndBlockedTags(const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
	for (const FARTAbilityTagRelationshipItem& Relationship : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Relationship.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Relationship.ActivationRequiredTags);
			}
 
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Relationship.ActivationBlockedTags);
			}
		}
	}
}

void UARTAbilityTagRelationship::GetAbilityCancelTags(const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutAbilityCancelTags)
{
	for (const FARTAbilityTagRelationshipItem& Relationship : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Relationship.AbilityTag))
		{
			if(OutAbilityCancelTags)
			{
				OutAbilityCancelTags->AppendTags(Relationship.AbilityCancelTags);
			}
		}
	}
}
