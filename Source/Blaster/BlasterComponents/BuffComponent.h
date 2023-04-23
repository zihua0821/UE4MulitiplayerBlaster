#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

/**
 * BUFF组件
 * 用于处理玩家BUFF
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;

	/**
	 * 治愈
	 * @param HealAmount 治愈量
	 * @param HealingTime 治愈时间
	 */
	void Heal(float HealAmount, float HealingTime);

	/**
	 * 恢复护盾
	 * @param ShieldAmount 护盾恢复量
	 * @param ReplenishTime 恢复时间
	 */
	void ReplenishShield(float ShieldAmount, float ReplenishTime);

	/**
	 * 速度BUFF
	 * @param BuffBaseSpeed BUFF行走速度
	 * @param BuffCrouchSpeed BUFF蹲伏行走速度
	 * @param BuffTime BUFF时间
	 */
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);

	/**
	 * 跳跃BUFF
	 * @param BuffJumpVelocity BUFF跳跃速度
	 * @param BuffTime BUFF时间
	 */
	void BuffJump(float BuffJumpVelocity, float BuffTime);

	/**
	 * 恢复速度
	 * @param BaseSpeed 基础行走速度
	 * @param CrouchSpeed 基础蹲伏行走速度
	 */
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	/**
	 * 每帧治愈
	 * @param Velocity 基础跳跃速度
	 */
	void SetInitialJumpVelocity(float Velocity);
	
protected:
	
	virtual void BeginPlay() override;

	/**
	 * 每帧治愈
	 * @param DeltaTime 帧间隔时间
	 */
	void HealRampUp(float DeltaTime);

	/**
	 * 每帧恢复护盾
	 * @param DeltaTime 帧间隔时间
	 */
	void ShieldRampUp(float DeltaTime);
	
private:

	//玩家角色指针
	UPROPERTY()
	ABlasterCharacter* Character;

	//是否正在治愈
	bool bHealing = false;
	//治愈速率
	float HealingRate = 0.f;
	//治愈量
	float AmountToHeal = 0.f;

	//是否正在恢复护盾
	bool bReplenishingShield = false;
	//护盾恢复速率
	float ShieldReplenishRate = 0.f;
	//护盾恢复量
	float ShieldReplenishAmount = 0.f;

	//加速BUFF计时器
	FTimerHandle SpeedBuffTimer;

	//重置速度
	void ResetSpeeds();

	//初始基础速度
	float InitialBaseSpeed;

	//初始蹲伏行走速度
	float InitialCrouchSpeed;

	/**
	 * 加速BUFF
	 * 多播
	 * @param BaseSpeed 基础移动速度
	 * @param CrouchSpeed 蹲伏行走速度
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	//跳跃BUFF计时器
	FTimerHandle JumpBuffTimer;

	//重置跳跃
	void ResetJump();

	//基础跳跃速度
	float InitialJumpVelocity;

	/**
	 * 加速BUFF
	 * 多播
	 * @param JumpVelocity 跳跃速度
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
