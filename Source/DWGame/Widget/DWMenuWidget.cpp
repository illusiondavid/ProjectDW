#include "DWMenuWidget.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "DWGame/NetWork/DWNetSubsystem.h"
#include "DWGame/Settings/DWGameSettings.h"


DEFINE_LOG_CATEGORY(LogDWMenuWidget);

void UDWMenuWidget::SetMenu()
{

	//获取网络子系统
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		NetSubsystem = GameInstance->GetSubsystem<UDWNetSubsystem>();
	}
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(!World)
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if(!PlayerController)
	{
		return;	
	}

	//设置界面输入模式
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	//显示鼠标
	PlayerController->SetShowMouseCursor(true);

	if(NetSubsystem)
	{
		NetSubsystem->OnCreateDwNetSessionComplete.AddDynamic(this,&UDWMenuWidget::OnCreateDWSession);
		NetSubsystem->OnStartDwNetSessionComplete.AddDynamic(this,&UDWMenuWidget::UDWMenuWidget::OnStartDWSession);
		NetSubsystem->OnFindDwNetSessionComplete.AddUObject(this,&UDWMenuWidget::OnFindDWSession);
		NetSubsystem->OnJoinDwNetSessionComplete.AddUObject(this,&UDWMenuWidget::OnJoinDWSession);
		NetSubsystem->OnDestroyDwNetSessionComplete.AddDynamic(this,&UDWMenuWidget::OnDestroyDWSession);
	}
}

void UDWMenuWidget::BeginDestroy()
{
	Super::BeginDestroy();
}


void UDWMenuWidget::OnCreateRoomButtonClicked()
{
	if(!NetSubsystem)
	{
		UE_LOG(LogDWMenuWidget, Error, TEXT("NetSubsystem is null!"));
		return;
	}

	//界面销毁时重置输入模式
	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PlayerController = /*World->GetFirstPlayerController()*/GetOwningPlayer())
		{
			FInputModeGameOnly InputModeGameOnly;
			PlayerController->SetInputMode(InputModeGameOnly);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	RemoveFromParent();
	
	NetSubsystem->OpenLobby();
}


void UDWMenuWidget::OnJoinRoomButtonClicked()
{
	if(!NetSubsystem)
	{
		return;
	}
	NetSubsystem->JoinLobby();
}

void UDWMenuWidget::OnCreateDWSession(bool bSuccessful)
{
	if(!bSuccessful)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Red,FString::Printf(TEXT("Create Session fail")));
		}
		return;
	}
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("Create Session Success")));
	}
	if(UWorld* World = GetWorld())
	{
		UDWGameSettings* DWGameSettings = UDWGameSettings::Get();
		//以ListenServer方式开启地图
		FString LobbyPath = FString::Printf(TEXT("%s?listen"), *DWGameSettings->LobbyURL);
		UE_LOG(LogDWNetSubsystem, Log, TEXT("Travel to Lobby map : %s"),*LobbyPath);
		//开启大厅地图
		bool bTravelSuccess = World->ServerTravel(LobbyPath);
		if(!bTravelSuccess)
		{
			UE_LOG(LogDWNetSubsystem, Error, TEXT("Travel to Lobby map false"));
		}
	}
}

void UDWMenuWidget::OnStartDWSession(bool bSuccessful)
{
}

void UDWMenuWidget::OnFindDWSession(const TArray<FOnlineSessionSearchResult>& DWSessionSearchResults, bool bSuccessful)
{
	if(!NetSubsystem)
	{
		return;
	}
	for(auto& SearchResult : DWSessionSearchResults )
	{
		FString ValueOfSettings;
		SearchResult.Session.SessionSettings.Get(FName("MatchType"),ValueOfSettings);
		if(ValueOfSettings == UDWGameSettings::Get()->MatchType)
		{
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("开始加入房间")));
			}
			UE_LOG(LogDWNetSubsystem, Error, TEXT("开始加入房间"));
			NetSubsystem->JoinNetSession(SearchResult);
			return;
		}
	}
}

void UDWMenuWidget::OnJoinDWSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if(!OnlineSubsystem)
	{
		return;
	}
	IOnlineSessionPtr OnlineSessionPtr = OnlineSubsystem->GetSessionInterface();
	if(!OnlineSessionPtr.IsValid())
	{
		return;
	}
	FString URLAddress;
	OnlineSessionPtr->GetResolvedConnectString(NAME_GameSession,URLAddress);
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if(PlayerController)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("开始传送")));
		}
		UE_LOG(LogDWNetSubsystem, Error, TEXT("开始传送"));
		
		PlayerController->ClientTravel(URLAddress,ETravelType::TRAVEL_Absolute);
	}
}

void UDWMenuWidget::OnDestroyDWSession(bool bSuccessful)
{
}

void UDWMenuWidget::DestroyMenuWidget()
{
}
