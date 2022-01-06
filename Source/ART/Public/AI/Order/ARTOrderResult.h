// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EARTOrderResult : uint8
{
	/** The order was fulfilled. */
	SUCCEEDED,

	/** The order was canceled. */
	CANCELED,

	/** The order has failed. */
	FAILED,
};
