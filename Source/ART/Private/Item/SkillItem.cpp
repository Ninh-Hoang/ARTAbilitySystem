// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SkillItem.h"

#include "ARTAssetManager.h"

#define LOCTEXT_NAMESPACE "SkillItem"
USkillItem::USkillItem()
{
	ItemType = UARTAssetManager::SkillItemType;
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
}
#undef LOCTEXT_NAMESPACE
