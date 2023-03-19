#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "DWMenuWidget.generated.h"

class UDWNetSubsystem;
class UButton;

DECLARE_LOG_CATEGORY_EXTERN(LogDWMenuWidget, Log, All);

UCLASS()
class DWGAME_API UDWMenuWidget: public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetMenu();

	UFUNCTION(BlueprintCallable)
	void OnCreateRoomButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnJoinRoomButtonClicked();

	UFUNCTION()
	void OnCreateDWSession(bool bSuccessful);

	UFUNCTION()
	void OnStartDWSession(bool bSuccessful);
	
	void OnFindDWSession(const TArray<FOnlineSessionSearchResult>& DWSessionSearchResults,bool bSuccessful);
	
	void OnJoinDWSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroyDWSession(bool bSuccessful);
	
protected:
	
	virtual void BeginDestroy() override;
	
private:
	
	void DestroyMenuWidget();

	UDWNetSubsystem* NetSubsystem;
};
