#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 * 返回菜单HUD控件
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:

	/**
	 * 设置菜单
	 */
	void MenuSetup();

	/**
	 * 关闭菜单
	 */
	void MenuTearDown();

protected:
	
	//控件初始化
	virtual bool Initialize() override;

	/**
	 * 会话销毁回调
	 */
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	/**
	 * 玩家离开游戏回调
	 */
	UFUNCTION()
	void OnPlayerLeftGame();
private:

	//返回按钮绑定
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	//返回按钮点击事件
	UFUNCTION()
	void ReturnButtonClicked();

	//多人会话子系统指针
	UPROPERTY()
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//玩家控制器指针
	UPROPERTY()
	APlayerController* PlayerController;
};
