// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ARTGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>

UARTGameInstance::UARTGameInstance()
{
	//OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGameInfoInstance::OnCreateSessionComplete);
	//OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UGameInfoInstance::OnStartOnlineGameComplete);
}

void UARTGameInstance::Init()
{
	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get())
	{
		SessionInterface = SubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			//bind delegates
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &UARTGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
				this, &UARTGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UARTGameInstance::OnJoinSessionComplete);
		}
	}
}

bool UARTGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId,
                                   FName SessionName, bool bIsLAN, bool bIsPresence,
                                   int32 MaxNumPlayers)
{
	return true;
}

void UARTGameInstance::OnCreateSessionComplete(FName ServerName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete, Succeeded: %d"), Succeeded);

	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/Level/Lobby?Listen");
	}
}

void UARTGameInstance::OnFindSessionsComplete(bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete, Succeeded: %d"), Succeeded);

	if (Succeeded)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		UE_LOG(LogTemp, Warning, TEXT("Search Results, Server count: %i"), SearchResults.Num());

		if (SearchResults.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("Joining Server"));

			SessionInterface->JoinSession(0, "My Session", SearchResults[0]);
		}
	}
}

void UARTGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("Joining Session Complete, SessionName: %s"), *SessionName.ToString());

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = " ";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);

		if (JoinAddress != "")
		{
			PC->ClientTravel(JoinAddress, TRAVEL_Absolute);
		}
	}
}

void UARTGameInstance::CreateServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Server"));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 3;

	SessionInterface->CreateSession(0, FName("My Session"), SessionSettings);
}

void UARTGameInstance::JoinServer()
{
	if (SessionInterface->HasPresenceSession())
	{
		SessionInterface->DestroySession(FName("My Session"));
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}
