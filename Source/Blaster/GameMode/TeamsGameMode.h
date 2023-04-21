#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 多人团队游戏模式
 * 继承自多人游戏
 * 玩家分为两个队伍，队伍间相互攻击，无队伍伤害
 * 负责玩家登入登出控制 玩家的淘汰与重生 过滤阻止队友伤害
 * 
 * 派生蓝图类Tips
 * 继承多人游戏的蓝图配置
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	//覆写玩家登入
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//覆写玩家登出
	virtual void Logout(AController* Exiting) override;
	//覆写伤害计算
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	//覆写玩家淘汰
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
protected:
	//覆写比赛开始
	virtual void HandleMatchHasStarted() override;
};
