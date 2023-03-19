// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DWNetSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDWNetSubsystem, Log, All);

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCreateDWNetSessionComplete,bool,bSuccessful );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnStartDWNetSessionComplete,bool,bSuccessful );
DECLARE_MULTICAST_DELEGATE_TwoParams( FOnFindDWNetSessionComplete,const TArray<FOnlineSessionSearchResult>& DWSessionSearchResults,bool bSuccessful );
DECLARE_MULTICAST_DELEGATE_OneParam( FOnJoinDWNetSessionComplete,EOnJoinSessionCompleteResult::Type Result );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnDestroyDWNetSessionComplete,bool,bSuccessful );

UCLASS()
class DWGAME_API UDWNetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UDWNetSubsystem();

	void OpenLobby();

	void JoinLobby();

	//创建网络会话
	void CreateNetSession(int32 ConnectionNum, FString MatchType);

	//查找网络会话
	void FindNetSessions(int32 MaxResultsNum);

	//加入网络会话
	void JoinNetSession(const FOnlineSessionSearchResult& OnlineSessionSearchResult);

	//开启网络会话
	void StartNetSession();

	//销毁网络会话
	void DestroyNetSession();

	FOnCreateDWNetSessionComplete OnCreateDwNetSessionComplete;
	FOnStartDWNetSessionComplete OnStartDwNetSessionComplete;
	FOnFindDWNetSessionComplete OnFindDwNetSessionComplete;
	FOnJoinDWNetSessionComplete OnJoinDwNetSessionComplete;
	FOnDestroyDWNetSessionComplete OnDestroyDwNetSessionComplete;
	
protected:

	/**********回调函数**********/
	//网络会话创建完成回调
	void OnCreateNetSessionCompleted(FName NetSessionName, bool bSuccessful);

	//网络会话查找完成回调
	void OnFindNetSessionsCompleted(bool bSuccessful);

	//网络会话加入完成回调
	void OnJoinNetSessionCompleted(FName NetSessionName, EOnJoinSessionCompleteResult::Type JoinSessionCompleteResult);

	//网络会话开启完成回调
	void OnStartNetSessionCompleted(FName NetSessionName, bool bSuccessful);

	//网络会话销毁完成回调
	void OnDestroyNetSessionCompleted(FName NetSessionName, bool bSuccessful);

	
private:
	IOnlineSessionPtr OnlineSessionPtr;

	/**********网络会话回调委托**********/
	//网络会话创建完成回调委托
	FOnCreateSessionCompleteDelegate CreateNetSessionCompleteDelegate;
	
	//网络会话查找完成回调委托
	FOnFindSessionsCompleteDelegate FindNetSessionsCompleteDelegate;
	
	//网络会话加入完成回调委托
	FOnJoinSessionCompleteDelegate JoinNetSessionCompleteDelegate;

	//网络会话开启完成回调委托
	FOnStartSessionCompleteDelegate StartNetSessionCompleteDelegate;
	
	//网络会话销毁完成回调委托
	FOnDestroySessionCompleteDelegate DestroyNetSessionCompleteDelegate;

	/**********网络会话委托Handle**********/
	//网络会话创建完成Handle
	FDelegateHandle CreateNetSessionCompleteHandle;
	
	//网络会话查找完成Handle
	FDelegateHandle FindNetSessionsCompleteHandle;
	
	//网络会话加入完成Handle
	FDelegateHandle JoinNetSessionCompleteHandle;
	
	//网络会话开启完成Handle
	FDelegateHandle StartNetSessionCompleteHandle;
	
	//网络会话销毁完成Handle
	FDelegateHandle DestroyNetSessionCompleteHandle;

	//上一次网络会话的设置
	TSharedPtr<FOnlineSessionSettings> NetSessionSettings;

	//上一次网络会话查找结果
	TSharedPtr<FOnlineSessionSearch> NetSessionSearch;
	
	bool bCreateNetSessionOnDestroy; 
};
