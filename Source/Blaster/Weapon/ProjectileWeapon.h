#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

/**
 * 射弹武器
 * 继承自武器基类
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	
	//重写开火
	virtual void Fire(const FVector& HitTarget) override; 

private:

	//子弹子类
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;

	//服务器倒带子弹子类
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
