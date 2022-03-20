#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Defense.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Defense : public UARTAttributeSetBase
{
	GENERATED_BODY()
	
public:
	UARTAttributeSet_Defense();
	
	//Armor
	UPROPERTY(BlueprintReadOnly, Category = "Armor", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Defense, Armor)

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Defense, Armor, OldArmor);
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};