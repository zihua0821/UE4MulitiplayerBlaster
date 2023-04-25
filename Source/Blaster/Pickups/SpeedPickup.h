#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 速度BUFF拾取物
 * 继承自Pickup
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
	
protected:
	//重写覆盖函数
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
	
	//基础BUFF速度
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	//蹲伏BUFF速度
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	//BUFF时间
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;
};
