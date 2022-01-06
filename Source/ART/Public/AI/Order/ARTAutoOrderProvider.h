// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ARTOrderData.h"
#include "ARTOrderTypeWithIndex.h"
#include "ARTAutoOrderProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UARTAutoOrderProvider : public UInterface
{
	GENERATED_BODY()
};

/**
* Implement this interface on an 'UActorComponent' to provide auto orders that should be issued to this unit
* automatically.
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutoOrderUpdate, const FARTOrderTypeWithIndex&, AutoOrder);

class ART_API IARTAutoOrderProvider
{
	GENERATED_BODY()

public:
	/**
	* Should fill the passed array with orders that should be issued to this unit automatically.
	*/
	
	UFUNCTION(Category = Order, BlueprintNativeEvent, BlueprintCallable)
	void GetAutoOrders(TArray<FARTOrderTypeWithIndex>& OutAutoOrders);

	virtual FOnAutoOrderUpdate* GetAutoOrderAddDelegate() = 0;
	virtual FOnAutoOrderUpdate* GetAutoOrderRemoveDelegate() = 0;
};