#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 霰弹枪武器
 * 继承自射线检测武器
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:

	/**
	 * 霰弹枪开火
	 * @param HitTargets 攻击目标数组
	 */
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);

	/**
	 * 霰弹枪开火
	 * @param HitTarget 攻击目标
	 * @param HitTargets 攻击目标数组
	 */
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
	
private:

	//弹丸数量
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};
