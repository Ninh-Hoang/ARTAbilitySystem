// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ARTGameInstance.generated.h"

/**
 * 
 */

UCLASS()
class ART_API UARTGameInstance : public UGameInstance
{
	GENERATED_BODY()


protected:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/* Delegate called when session created */
	//FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;

	/* Delegate called when session started */
	//FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

public:
	UARTGameInstance();

protected:
	virtual void Init() override;

	bool HostSession(TSharedPtr<const FUniqueNetId> UserId,
	                 FName SessionName, bool bIsLAN, bool bIsPresence,
	                 int32 MaxNumPlayers);

	virtual void OnCreateSessionComplete(FName ServerName, bool Succeeded);
	virtual void OnFindSessionsComplete(bool Succeeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable)
	void CreateServer();

	UFUNCTION(BlueprintCallable)
	void JoinServer();
};
