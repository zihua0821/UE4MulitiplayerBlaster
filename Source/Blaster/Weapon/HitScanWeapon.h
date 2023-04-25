#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;

/**
 * 射线检测武器
 * 继承自武器基类
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	
	//重写开火
	virtual void Fire(const FVector& HitTarget) override;
	
protected:

	/**
	 * 射线检测
	 * @param TraceStart 起始位置
	 * @param HitTarget 目标位置
	 * @param OutHit 检测结果
	 */
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	//受击音效
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	//子弹爆炸粒子
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

private:

	//子弹轨迹粒子
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	//枪口火焰粒子
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//开火音效
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
