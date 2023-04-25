#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 步枪子弹
 * 继承自子弹基类
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileBullet();

	//编辑器函数
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif
	
protected:

	//重写碰撞
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
};
