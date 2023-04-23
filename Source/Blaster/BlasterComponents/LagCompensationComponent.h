#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterPlayerController;

/**
 * 盒子信息
 * 盒子位置 旋转 范围
 */
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

/**
 * 帧数据
 * 时间 盒子信息Map 玩家角色指针
 */
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter* Character;
	
};

/**
 * 服务器倒带结果
 * 是否命中 是否爆头
 */
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

/**
 * 霰弹枪服务器倒带结果
 * 命中信息Map 爆头信息Map
 */
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;

};

/**
 * 延迟补偿组件
 * 处理客户端预测 服务器倒带 爆头命中
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 显示帧数据
	 * 仅测试使用
	 * @param Package 帧包的引用
	 * @param Color 显示的颜色
	 */
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	
	/**
	 * 服务器倒带算法
	 * @param HitCharacter 受击玩家指针
	 * @param TraceStart 射线起始位置
	 * @param HitLocation 射线命中位置
	 * @param HitTime 命中时间
	 */
	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	/**
	 * 射弹武器服务器倒带算法
	 * @param HitCharacter 受击玩家指针
	 * @param TraceStart 射线起始位置
	 * @param InitialVelocity 发射速度
	 * @param HitTime 命中时间
	 */
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/**
	 * 霰弹枪服务器倒带算法
	 * @param HitCharacters 受击玩家指针数组
	 * @param TraceStart 射线起始位置
	 * @param HitLocations 射线命中位置
	 * @param HitTime 命中时间
	 */
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations, 
		float HitTime);

	/**
	 * 服务器请求得分
	 * ServerRpc
	 * @param HitCharacter 受击玩家指针
	 * @param TraceStart 射线起始位置
	 * @param HitLocation 射线命中位置
	 * @param HitTime 命中时间
	 */
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	/**
	 * 射弹武器请求得分
	 * ServerRpc
	 * @param HitCharacter 受击玩家指针
	 * @param TraceStart 射线起始位置
	 * @param InitialVelocity 发射速度
	 * @param HitTime 命中时间
	 */
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/**
	 * 霰弹枪请求得分
	 * ServerRpc
	 * @param HitCharacters 受击玩家指针数组
	 * @param TraceStart 射线起始位置
	 * @param HitLocations 射线命中位置
	 * @param HitTime 命中时间
	 */
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
protected:
	virtual void BeginPlay() override;

	/**
	 * 储存帧数据
	 * @param Package 帧包的引用
	 */
	void SaveFramePackage(FFramePackage& Package);

	/**
	 * 帧数据之间进行插值
	 * @return 插值后的帧数据
	 * @param OlderFrame 较远的数据
	 * @param YoungerFrame 较近的数据
	 * @param HitTime 命中时间
	 */
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	/**
	 * 缓存盒子位置
	 * @param HitCharacter 受击的角色
	 * @param OutFramePackage 帧数据的引用
	 */
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	/**
	 * 移动盒子
	 * @param HitCharacter 受击的角色
	 * @param Package 帧数据的引用
	 */
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	/**
	 * 重新设置盒子
	 * @param HitCharacter 受击的角色
	 * @param Package 帧数据的引用
	 */
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	/**
	 * 启用角色碰撞
	 * @param HitCharacter 角色指针
	 * @param CollisionEnabled 碰撞类别
	 */
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	/**
	 * 储存帧数据
	 */
	void SaveFramePackage();

	/**
	 * 验证是否命中
	 * @param Package 帧数据引用
	 * @param HitCharacter 受击玩家
	 * @param TraceStart 射线起始位置
	 * @param HitLocation 射线命中位置
	 */
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package, 
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation);

	/**
	 * 验证射弹武器是否命中
	 * @param Package 帧数据引用
	 * @param HitCharacter 受击玩家
	 * @param TraceStart 射线起始位置
	 * @param InitialVelocity 发射速度
	 * @param HitTime 命中时间
	 */
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/**
	 * 验证霰弹枪是否命中
	 * @param FramePackages 帧数据数组引用
	 * @param TraceStart 射线起始位置
	 * @param HitLocations 射线命中位置数组
	 */
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

	/**
	 * 验证射弹武器是否命中
	 * @param HitCharacter 受击玩家
	 * @param HitTime 击中时间
	 */
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

private:

	//玩家角色指针
	UPROPERTY()
	ABlasterCharacter* Character;

	//玩家控制器指针
	UPROPERTY()
	ABlasterPlayerController* Controller;

	//帧数据双链表
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//最大回溯时间
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
public:	

		
};
