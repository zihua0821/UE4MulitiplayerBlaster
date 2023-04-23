#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

//高Ping代理 动态多播单参数
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ABlasterHUD;
class UReturnToMainMenu;
class ABlasterGameMode;
class UCharacterOverlay;
class ABlasterPlayerState;
class ABlasterGameState;

/**
 * 玩家控制器
 * 更新HUD 检查Ping 设置游戏信息
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	/**
	 * 设置HUD生命值
	 * @param Health 生命值
	 * @param MaxHealth 最大生命值
	 */
	void SetHUDHealth(float Health, float MaxHealth);
	/**
	 * 设置HUD护盾值
	 * @param Shield 护盾值
	 * @param MaxShield 最大护盾值
	 */
	void SetHUDShield(float Shield, float MaxShield);
	/**
	 * 设置HUD得分
	 * @param Score 得分
	 */
	void SetHUDScore(float Score);
	/**
	 * 设置HUD被击败数
	 * @param Defeats 被击败数
	 */
	void SetHUDDefeats(int32 Defeats);
	/**
	 * 设置HUD武器子弹数
	 * @param Ammo 子弹数
	 */
	void SetHUDWeaponAmmo(int32 Ammo);
	/**
	 * 设置HUD玩家携带子弹数
	 * @param Ammo 子弹数
	 */
	void SetHUDCarriedAmmo(int32 Ammo);
	/**
	 * 设置HUD倒计时
	 * @param CountdownTime 倒计时时间
	 */
	void SetHUDMatchCountdownText(float CountdownTime);
	/**
	 * 设置HUD公告
	 * @param CountdownTime 倒计时时间
	 */
	void SetHUDAnnouncementCountdown(float CountdownTime);
	/**
	 * 设置HUD手榴弹数量
	 * @param Grenades 手榴弹数量
	 */
	void SetHUDGrenades(int32 Grenades);
	/**
	 * 设置HUDPing
	 * @param Ping Ping值
	 */
	void SetHUDPingText(int32 Ping);

	/**
	 * 当控制器控制Pawn时
	 * @param InPawn 控制的Pawn
	 */
	virtual void OnPossess(APawn* InPawn) override;
	//Tick
	virtual void Tick(float DeltaSeconds) override;
	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 隐藏团队得分
	 */
	void HideTeamScores();
	/**
	 * 初始化团队得分
	 */
	void InitTeamScores();
	/**
	 * 设置HUD红队得分
	 * @param RedScore 红队得分
	 */
	void SetHUDRedTeamScore(int32 RedScore);
	/**
	 * 设置HUD蓝队得分
	 * @param BlueScore 蓝队得分
	 */
	void SetHUDBlueTeamScore(int32 BlueScore);

	/**
	 * 获取服务器时间
	 * @return float
	 * 虚函数
	 */
	virtual float GetServerTime();
	//在此玩家控制器的视口/网络连接与此玩家控制器关联后调用。
	virtual void ReceivedPlayer() override;
	
	/**
	 * 游戏状态设置时
	 * @param State 游戏状态
	 * @param bTeamsMatch 是否是团队游戏
	 */
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	/**
	 * 处理游戏开始
	 * @param bTeamsMatch 是否是团队游戏
	 */
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	/**
	 * 处理游戏冷却
	 */
	void HandleCooldown();
	//单程时间
	float SingleTripTime = 0.f;
	//声明代理
	FHighPingDelegate HighPingDelegate;

	/**
	 * 广播玩家淘汰信息
	 * @param Attacker 攻击者
	 * @param Victim 被攻击者
	 */
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	
protected:
	
	virtual void BeginPlay() override;

	/**
	 * 设置HUD时间
	 */
	void SetHUDTime();
	/**
	 * 轮询初始化
	 */
	void PollInit();

	//绑定输入
	virtual void SetupInputComponent() override;

	/**
	 * 请求服务器时间
	 * @param TimeOfClientRequest 客户端请求的时间
	 * ServerRPC
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/**
	 * 请求服务器时间
	 * @param TimeOfClientRequest 客户端请求的时间
	 * @param TimeServerReceivedClientRequest 服务器收到客户端请求的时间
	 * ClientRPC
	 */
	UFUNCTION(Server, Reliable)
	void ClientRequestServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//客户端服务器之间时间间隔
	float ClientServerDelta = 0.f;
	
	//时间同步频率
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	//时间同步计时
	float TimeSyncRunningTime = 0.f;
	/**
	 * 检查同步时间
	 * @param DeltaTime 增量时间
	 */
	void CheckTimeSync(float DeltaTime);

	/**
	 * 检查游戏状态
	 * ServerRPC
	 */
	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	/**
	 * 玩家中途加入游戏
	 * ClientRPC
	 * @param StateOfMatch 游戏状态
	 * @param Warmup 热身时间
	 * @param Match 游戏时间
	 * @param Cooldown 冷却时间
	 * @param StartingTime 游戏开始的时间
	 */
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	/**
	 * 高Ping警告
	 */
	void HighPingWarning();
	/**
	 * 停止高Ping警告
	 */
	void StopHighPingWarning();
	/**
	 * 检查Ping
	 * @param DeltaTime 增量时间
	 */
	void CheckPing(float DeltaTime);
	/**
	 * 显示Ping
	 * @param DeltaTime 增量时间
	 */
	void ShowPing(float DeltaTime);
	
	/**
	 * 显示返回菜单
	 */
	void ShowReturnToMainMenu();

	/**
	 * 淘汰公告
	 * ClientRPC
	 * @param Attacker 攻击者
	 * @param Victim 被攻击者
	 */
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	//是否显示团队比分 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	//是否显示团队比分复制更新时触发的回调
	UFUNCTION()
	void OnRep_ShowTeamScores();

	/**
	 * 获取公告信息
	 * @param Players 玩家状态数组
	 * @return FString
	 */
	FString GetInfoText(const TArray<ABlasterPlayerState*>& Players);

	/**
	 * 获取团队公告信息
	 * @param BlasterGameState 游戏状态
	 * @return FString
	 */
	FString GetTeamsInfoText(ABlasterGameState* BlasterGameState);
	
private:

	//HUD指针
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	//返回菜单组件子类
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuWidget;

	//返回菜单指针
	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenu;

	//返回菜单是否打开
	bool bReturnToMainMenuOpen = false;

	//游戏模式指针
	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	//游戏已经开始的时间
	float LevelStartingTime = 0.f;
	//游戏时间
	float MatchTime = 0.f;
	//热身时间
	float WarmupTime = 0.f;
	//冷却时间
	float CooldownTime = 0.f;
	//倒计时
	uint32 CountdownInt = 0;

	//游戏状态 复制更新触发回调
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	/**
	 * 游戏状态复制更新时触发的回调
	 */
	UFUNCTION()
	void OnRep_MatchState();

	//玩家覆盖层HUD控件指针
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	//生命值HUD
	float HUDHealth;
	//是否已经初始化生命值
	bool bInitializeHealth = false;
	//最大生命值HUD
	float HUDMaxHealth;
	//得分HUD
	float HUDScore;
	//是否已经初始化得分
	bool bInitializeScore = false;
	//被击败数HUD
	int32 HUDDefeats;
	//是否已经初始化被击败数
	bool bInitializeDefeats = false;
	//手榴弹数量HUD
	int32 HUDGrenades;
	//是否已经初始化手榴弹数量
	bool bInitializeGrenades = false;
	//护盾值HUD
	float HUDShield;
	//是否已经初始化护盾值
	bool bInitializeShield = false;
	//最大护盾值HUD
	float HUDMaxShield;
	//玩家携带的子弹HUD
	float HUDCarriedAmmo;
	//是否已经初始化玩家携带的子弹
	bool bInitializeCarriedAmmo = false;
	//武器子弹HUD
	float HUDWeaponAmmo;
	//是否已经初始化武器子弹
	bool bInitializeWeaponAmmo = false;
	//PingHUD
	int32 HUDPing;
	//是否已经初始化Ping
	bool bInitializePing = false;
	//显示Ping频率
	UPROPERTY(EditAnywhere)
	float ShowPingFrequency = 1.f;
	//Ping时间
	float ShowPingRunningTime = 0.f;
	//高Ping时间
	float HighPingRunningTime = 0.f;
	//高Ping持续时间
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	//Ping动画持续时间
	float PingAnimationRunningTime = 0.f;
	//检测Ping的频率
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	//向服务器请求Ping ServerRPC
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	//高Ping阈值
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};



