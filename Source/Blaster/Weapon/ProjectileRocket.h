#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;

/**
 * 火箭弹
 * 继承自子弹基类
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	//重写摧毁
	virtual void Destroyed() override;
	
protected:
	
	//重写碰撞事件
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	//重写开始
	virtual void BeginPlay() override;

	//火箭弹飞行音效
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	//音效组件
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	//音效范围衰减
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	//火箭弹移动组件
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
	
private:
	
};

