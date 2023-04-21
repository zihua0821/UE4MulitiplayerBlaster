#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 游戏大厅游戏模式
 * 继承自GameMode类 拥有基础的多人游戏功能
 * 玩家大于两人时开始游戏
 *
 * 派生蓝图类Tips
 * 使用玩家Pawn
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
