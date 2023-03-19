#include "DWGameSettings.h"
#include "UObject/UObjectIterator.h"

UDWGameSettings::UDWGameSettings()
	:ConnectionNum(4)
	,MaxFindResultsNum(10000)
	,MatchType("FreeForAll")
	,LobbyURL("/Game/Maps/LobbyRoom")
{
}

UDWGameSettings* UDWGameSettings::Get()
{
	static UDWGameSettings* Instance;
	if (!Instance)
	{
		for (const TObjectIterator<UDWGameSettings> Itr(RF_NoFlags); Itr;)
		{
			Instance = *Itr;
			break;
		}
		OverrideConfigByCommandLine(Instance);
	}
	return Instance;
}

void UDWGameSettings::OverrideConfigByCommandLine(UDWGameSettings* Instance)
{
	if (!Instance)
	{
		return;
	}

#if UE_BUILD_SHIPPING
	UE_LOG(LogDSGameSettings, Log, TEXT("UE_BUILD_SHIPPING pattern bForceEnableNetEncryption == true"));
	Instance->bForceEnableNetEncryption = true;
#endif
	
	//LobbyURL
	FString InLobbyURL;
	if (FParse::Value(FCommandLine::Get(), TEXT("LobbyURL="), InLobbyURL))
	{
		Instance->LobbyURL = InLobbyURL;
	}

}
