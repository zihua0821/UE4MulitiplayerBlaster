#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

/**
 * 游戏状态
 * 继承自GameState 拥有基础的多人游戏配置
 * 储存队伍信息 队伍玩家 队伍得分
 * 控制队伍分数更新
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 更新得分最高玩家
	 * @param ScoringPlayer 有分数更新的玩家状态
	 */
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	//分数最高玩家状态指针数组 复制变量
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/**
	 * 为红队添加一分 仅服务器调用
	 */
	void RedTeamScores();

	/**
	 * 为蓝队添加一分 仅服务器调用
	 */
	void BlueTeamScores();

	//红队玩家状态指针数组
	TArray<ABlasterPlayerState*> RedTeam;

	//蓝队玩家状态指针数组
	TArray<ABlasterPlayerState*> BlueTeam;

	//红队得分 复制变量 变量更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	
	//蓝队得分 复制变量 变量更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	/**
	 * 红队得分复制更新时触发的回调 仅客户端触发
	 */
	UFUNCTION()
	void OnRep_RedTeamScore();

	/**
	 * 蓝队得分复制更新时触发的回调 仅客户端触发
	 */
	UFUNCTION()
	void OnRep_BlueTeamScore();
private:
	//最高得分
	float TopScore = 0.f;
};
