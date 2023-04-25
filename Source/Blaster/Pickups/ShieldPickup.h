#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 护盾恢复拾取物
 * 继承自Pickup
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()
protected:
	//重写覆盖函数
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	//护盾恢复量
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	//护盾恢复时间
	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};
