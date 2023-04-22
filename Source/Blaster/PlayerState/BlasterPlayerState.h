#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 玩家状态
 * 继承自PlayerState
 * 内置玩家得分变量
 * 储存玩家信息 得分 被击败数 玩家队伍
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:

	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//覆写得分复制更新回调
	virtual void OnRep_Score() override;

	/**
	 * 被淘汰数复制更新回调
	 */
	UFUNCTION()
	virtual void OnRep_Defeats();

	/**
	 * 为玩家添加分数
	 * @param ScoreAmount 添加的分数 
	 */
	void AddToScore(float ScoreAmount);
	
	/**
	 * 为玩家添加被淘汰数
	 * @param DefeatsAmount 添加的被淘汰数 
	 */
	void AddToDefeats(int32 DefeatsAmount);
	
private:

	//玩家角色指针
	UPROPERTY()
	class ABlasterCharacter* Character;

	//玩家控制器指针
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	//玩家被淘汰数 复制变量 复制后触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	//玩家队伍 复制变量 复制后触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	/**
	 * 玩家队伍复制更新回调
	 */
	UFUNCTION()
	void OnRep_Team();
public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
};
