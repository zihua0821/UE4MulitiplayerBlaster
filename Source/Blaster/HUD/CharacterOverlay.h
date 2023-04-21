#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"
class UProgressBar;
class UTextBlock;
class UImage;

/**
 * 玩家覆盖层HUD控件
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:

	//血条
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	//血量文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	//护盾条
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	//护盾文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	//玩家分数文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	//红队分数文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	//蓝队分数文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	//比分分割线
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreSpacerText;

	//玩家被击败数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	//武器弹药数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	//玩家携带弹药数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	//游戏倒计时文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	//手雷数量
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	//高延迟图片
	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	//高延迟控件动画
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;

	//Ping文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;
};
