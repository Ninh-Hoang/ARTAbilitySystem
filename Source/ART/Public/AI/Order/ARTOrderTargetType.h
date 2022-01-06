// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
* The possible target types of an order.
*/
UENUM(BlueprintType)
enum class EARTTargetType : uint8
{
	/** No specific target (AOE, Aura or automatically applied to self). */
	NONE,

	/** Order needs an actor as target. */
	ACTOR,

	/** Order needs vector location as target. */
	LOCATION,

	/** Order needs vector location that is used together with the unit location as a direction. */
	DIRECTION,

	/** Can't be activated. */
	PASSIVE
};
