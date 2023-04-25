#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/**
 * 火箭弹的移动组件
 */
UCLASS()
class BLASTER_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
protected:
	
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	//重写处理爆炸，阻塞爆炸事件
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
};
