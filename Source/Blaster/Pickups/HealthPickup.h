#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 生命值恢复拾取物
 * 继承自Pickup
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
protected:
	//重写覆盖函数
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	//恢复量
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	//恢复时间
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	
};
