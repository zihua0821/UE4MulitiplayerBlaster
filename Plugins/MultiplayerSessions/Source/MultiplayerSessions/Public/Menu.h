// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 * 主菜单组件
 * 加入与创建会话
 * 拥有Host Join Quit三个按键
 * Quit在蓝图中实现
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	
	/**
	 * 设置菜单
	 * 蓝图调用
	 * @param NumberOfPublicConnections 会话连接数量，即最多玩家连接数，默认为4
	 * @param TypeOfMatch 匹配会话的字符串
	 * @param LobbyPath 游戏准备大厅的路径
	 */
	UFUNCTION(BlueprintCallable)
	void MenuSetup(
		int32 NumberOfPublicConnections = 4,
		FString TypeOfMatch = FString(TEXT("hello session!")),
		FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby"))
		);

protected:
	
	//控件初始化
	virtual bool Initialize() override;
	//销毁控件时触发
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//绑定到代理的事件回调
	//非动态代理不为UFUNCTION 
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
private:
	//绑定按钮
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	
	//按钮事件
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	/**
	 * 卸载组件
	 */
	void MenuTearDown();

	//多人子系统指针
	UPROPERTY()
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//最大连接数
	int32 NumPublicConnections{4};
	//匹配字符串
	FString MatchType{TEXT("hello session!")};
	//大厅地图路径
	FString PathToLobby{TEXT("")};
};
