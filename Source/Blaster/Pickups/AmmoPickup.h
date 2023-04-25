#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 弹药拾取物
 * 继承自Pickup
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
protected:
	//重写覆盖函数
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	//弹药数量
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	//武器类型
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};
