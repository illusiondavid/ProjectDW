#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DWGameSettings.generated.h"

class FString;


UCLASS(config = Game, meta = (DisplayName = "DWGame Settings"))
class DWGAME_API UDWGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UDWGameSettings();

	UFUNCTION(Blueprintpure, Category = "Settings")
	static UDWGameSettings* GetDSGameSettings() {return Get();}

	static UDWGameSettings* Get();

public:
	/*网络会话设置*/
	//会话连接数
	UPROPERTY(EditAnywhere,Category="NetSessionSettings")
	int32 ConnectionNum;

	//匹配类型
	UPROPERTY(EditAnywhere,Category="NetSessionSettings")
	FString MatchType;

	//大厅URL
	UPROPERTY(EditAnywhere,Category="NetSessionSettings")
	FString LobbyURL;

	//最多查找结果
	UPROPERTY(EditAnywhere,Category="NetSessionSettings")
	int32 MaxFindResultsNum;

private:
	// 使用命令行参数覆盖配置
	static void OverrideConfigByCommandLine(UDWGameSettings* Instance);
};
