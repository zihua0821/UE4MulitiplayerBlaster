#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 榴弹类
 * 继承自子弹基类
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileGrenade();
	//重写摧毁
	virtual void Destroyed() override;
protected:

	//重写开始
	virtual void BeginPlay() override;

	/**
	 * 榴弹弹跳
	 * @param ImpactResult 弹跳碰撞结果
	 * @param ImpactVelocity 弹跳速度
	 */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	
private:

	//弹跳音效
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
