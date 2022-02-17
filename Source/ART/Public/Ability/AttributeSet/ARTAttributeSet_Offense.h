#pragma once

#include "CoreMinimal.h"
#include "ARTAttributeSetBase.h"
#include "ARTAttributeSet_Offense.generated.h"

/**
 * 
 */
UCLASS()
class ART_API UARTAttributeSet_Offense : public UARTAttributeSetBase
{
	GENERATED_BODY()
	
public:
	UARTAttributeSet_Offense();
	
	//Health is important to check if alive or not, there it is in base class
	UPROPERTY(BlueprintReadOnly, Category = "AttackPower", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UARTAttributeSet_Offense, AttackPower)

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UARTAttributeSet_Offense, AttackPower, OldAttackPower);
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};