#include "DWNetSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "DWGame/Settings/DWGameSettings.h"

DEFINE_LOG_CATEGORY(LogDWNetSubsystem);
UDWNetSubsystem::UDWNetSubsystem()
	:bCreateNetSessionOnDestroy(false)
	,CreateNetSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateNetSessionCompleted))
	,FindNetSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindNetSessionsCompleted))
	,JoinNetSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinNetSessionCompleted))
	,StartNetSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnStartNetSessionCompleted))
	,DestroyNetSessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,&ThisClass::OnDestroyNetSessionCompleted))
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		OnlineSessionPtr = OnlineSubsystem->GetSessionInterface();
	}
}

void UDWNetSubsystem::OpenLobby()
{
	UDWGameSettings* DWGameSettings = UDWGameSettings::Get();
	CreateNetSession(DWGameSettings->ConnectionNum,DWGameSettings->MatchType);
}

void UDWNetSubsystem::JoinLobby()
{
	UDWGameSettings* DWGameSettings = UDWGameSettings::Get();
	FindNetSessions(UDWGameSettings::Get()->MaxFindResultsNum);
}

void UDWNetSubsystem::CreateNetSession(int32 ConnectionNum, FString MatchType)
{
	if(!OnlineSessionPtr)
	{
		UE_LOG(LogDWNetSubsystem, Error, TEXT("OnlineSessionPtr is null "));
		return;
	}
	
	//销毁已存在的NAME_GameSession
	FNamedOnlineSession* NamedOnlineSession = OnlineSessionPtr->GetNamedSession(NAME_GameSession);
	if(NamedOnlineSession)
	{
		bCreateNetSessionOnDestroy = true;
		DestroyNetSession();
	}

	CreateNetSessionCompleteHandle = OnlineSessionPtr->AddOnCreateSessionCompleteDelegate_Handle(CreateNetSessionCompleteDelegate);

	/*网络会话设置*/
	NetSessionSettings = MakeShareable(new FOnlineSessionSettings());
	//是否采用局域网
	if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("局域网联机")));
		}
		UE_LOG(LogDWNetSubsystem, Error, TEXT("局域网联机"));
		NetSessionSettings->bIsLANMatch = true;
	}
	else
	{
		NetSessionSettings->bIsLANMatch = false;
	}
	//公共连接数量
	NetSessionSettings->NumPublicConnections = ConnectionNum;
	//是否展示玩家的出场信息
	NetSessionSettings->bUsesPresence = true;
	//是否允许通过好友加入
	NetSessionSettings->bAllowJoinViaPresence = true;
	//是否允许进程中加入
	NetSessionSettings->bAllowJoinInProgress = true;
	//是否被推荐
	NetSessionSettings->bShouldAdvertise = true;
	
	NetSessionSettings->BuildUniqueId=1;
	
	NetSessionSettings->bUseLobbiesIfAvailable = true;
	//设置匹配和推荐方式
	NetSessionSettings->Set(FName("MatchType"),MatchType,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//创建会话
	UE_LOG(LogDWNetSubsystem, Log, TEXT("Begin create session "));
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bCreatedSuccess = OnlineSessionPtr->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,*NetSessionSettings);
	if(!bCreatedSuccess)
	{
		UE_LOG(LogDWNetSubsystem, Error, TEXT("Create session fail!"));
		//会话创建失败移除句柄
		OnlineSessionPtr->ClearOnCreateSessionCompleteDelegate_Handle(CreateNetSessionCompleteHandle);
		OnCreateDwNetSessionComplete.Broadcast(false);
	}
	else
	{
		UE_LOG(LogDWNetSubsystem, Log, TEXT("Create session success!"));
	}
}

void UDWNetSubsystem::FindNetSessions(int32 MaxResultsNum)
{
	if(!OnlineSessionPtr.IsValid())
	{
		return;
	}
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("Begin FindNetSessions,MaxResultsNum:%d"),MaxResultsNum));
	}

	FindNetSessionsCompleteHandle = OnlineSessionPtr->AddOnFindSessionsCompleteDelegate_Handle(FindNetSessionsCompleteDelegate);
	NetSessionSearch = MakeShareable(new FOnlineSessionSearch());
	NetSessionSearch->MaxSearchResults = MaxResultsNum;
	NetSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	if( IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("局域网搜寻联机")));
		}
		UE_LOG(LogDWNetSubsystem, Error, TEXT("局域网搜寻联机"));
	}
	NetSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if(GetWorld())
	{
		const ULocalPlayer* LocalPlayer =GetWorld()->GetFirstLocalPlayerFromController();
		bool bFound = OnlineSessionPtr->FindSessions( *LocalPlayer->GetPreferredUniqueNetId(),NetSessionSearch.ToSharedRef() );
		//如果没找到
		if(!bFound)
		{
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("Cannot find NetSessions")));
			}
			OnlineSessionPtr->ClearOnFindSessionsCompleteDelegate_Handle(FindNetSessionsCompleteHandle);
			OnFindDwNetSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
			UE_LOG(LogDWNetSubsystem, Log, TEXT("Cannot find NetSessions!"));
		}
	}
	
}

void UDWNetSubsystem::JoinNetSession(const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	if(!OnlineSessionPtr)
	{
		OnJoinDwNetSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinNetSessionCompleteHandle = OnlineSessionPtr->AddOnJoinSessionCompleteDelegate_Handle(JoinNetSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer =GetWorld()->GetFirstLocalPlayerFromController();
	bool bJoined = OnlineSessionPtr->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,OnlineSessionSearchResult);
	if(!bJoined)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("Join netsession fail")));
		}
		OnlineSessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinNetSessionCompleteHandle);
		OnJoinDwNetSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		UE_LOG(LogDWNetSubsystem, Log, TEXT("Join netsession fail!"));
	}
}

void UDWNetSubsystem::StartNetSession()
{
}

void UDWNetSubsystem::DestroyNetSession()
{
	if(!OnlineSessionPtr)
	{
		OnDestroyDwNetSessionComplete.Broadcast(false);
		return;
	}

	DestroyNetSessionCompleteHandle = OnlineSessionPtr->AddOnDestroySessionCompleteDelegate_Handle(DestroyNetSessionCompleteDelegate);
	if(!OnlineSessionPtr->DestroySession(NAME_GameSession))
	{
		OnlineSessionPtr->ClearOnDestroySessionCompleteDelegate_Handle(DestroyNetSessionCompleteHandle);
		OnDestroyDwNetSessionComplete.Broadcast(false);
	}
}

void UDWNetSubsystem::OnCreateNetSessionCompleted(FName NetSessionName, bool bSuccessful)
{
	if(OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnCreateSessionCompleteDelegate_Handle(CreateNetSessionCompleteHandle);
	}
	OnCreateDwNetSessionComplete.Broadcast(bSuccessful);
}

void UDWNetSubsystem::OnFindNetSessionsCompleted(bool bSuccessful)
{
	if(OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnFindSessionsCompleteDelegate_Handle(FindNetSessionsCompleteHandle);
	}
	if(NetSessionSearch->SearchResults.Num()<=0)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Green,FString::Printf(TEXT("SearchResults 为 0")));
		}
		OnFindDwNetSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
		UE_LOG(LogDWNetSubsystem, Log, TEXT("SearchResults 为 0"));
		return;
	}
	OnFindDwNetSessionComplete.Broadcast(NetSessionSearch->SearchResults,bSuccessful);
}

void UDWNetSubsystem::OnJoinNetSessionCompleted(FName NetSessionName,EOnJoinSessionCompleteResult::Type JoinSessionCompleteResult)
{
	if(OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinNetSessionCompleteHandle);
	}
	OnJoinDwNetSessionComplete.Broadcast(JoinSessionCompleteResult);
}

void UDWNetSubsystem::OnStartNetSessionCompleted(FName NetSessionName, bool bSuccessful)
{
}

void UDWNetSubsystem::OnDestroyNetSessionCompleted(FName NetSessionName, bool bSuccessful)
{
	if(OnlineSessionPtr)
	{
		OnlineSessionPtr->ClearOnDestroySessionCompleteDelegate_Handle(DestroyNetSessionCompleteHandle);
	}
	if(bSuccessful && bCreateNetSessionOnDestroy)
	{
		bCreateNetSessionOnDestroy = false;
		OpenLobby();
	}
	OnDestroyDwNetSessionComplete.Broadcast(bSuccessful);
}
