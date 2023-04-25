#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 跳跃BUFF拾取物
 * 继承自Pickup
 */
UCLASS()
class BLASTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()
protected:
	//重写覆盖函数
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	//跳跃BUFF速度
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	//BUFF时间
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
