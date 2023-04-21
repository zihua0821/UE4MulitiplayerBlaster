#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//拓展MatchState的namespace
namespace MatchState
{
	extern BLASTER_API const FName Cooldown; 
}

class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterPlayerState;

/**
 * 多人游戏模式
 * 继承自GameMode类 拥有基础的多人游戏功能
 * 玩家各自为营，可自由相互攻击
 * 负责玩家登入登出控制 玩家的淘汰与重生 游戏时间
 * 
 * 派生蓝图类Tips
 * 定义 游戏状态 玩家控制器 玩家状态 HUD 角色
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaSeconds) override;
	
	/**
	 * 玩家淘汰
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param VictimController 被淘汰的角色控制器
	 * @param AttackerController 发起攻击者的角色控制器
	 */
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	/**
	 * 玩家重生
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param ElimmedController 被淘汰的角色控制器
	 */
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	/**
	 * 玩家离开游戏
	 * @param PlayerLeaving 被淘汰的角色状态
	 */
	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);

	//热身等待时间
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	//游戏正式时间
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	//游戏结束等待时间
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	//游戏已经进行的时间，用于中途加入的玩家
	float LevelStartingTime = 0.f;

	/**
	 * 计算伤害
	 * @param Attacker 攻击者的角色控制器
	 * @param Victim 被攻击者的角色控制器
	 * @param BaseDamage 基础伤害
	 */
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	//是否开启团队游戏模式
	bool bTeamsMatch = false;
protected:
	virtual void BeginPlay() override;
	//游戏状态改变时的回调
	virtual void OnMatchStateSet() override;
private:
	//游戏倒计时
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime;}
};


