#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 拾取物基类
 */
UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

	//Tick
	virtual void Tick(float DeltaTime) override;

	//摧毁时
	virtual void Destroyed() override;

protected:

	//游戏开始时
	virtual void BeginPlay() override;

	/**
	 * 当有玩家重合时
	 * @param OverlappedComponent 重合组件
	 * @param OtherActor 重合的Actor
	 * @param OtherComp 重合的组件
	 * @param OtherBodyIndex 重合的Actor编号
	 * @param bFromSweep
	 * @param SweepResult 命中结果
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//旋转速度
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	
private:

	//球形组件
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	//拾取音效
	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;

	//静态网格体组件
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	//粒子系统组件
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

	//粒子系统
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;

	//延迟绑定计时器
	FTimerHandle BindOverlapTimer;
	//延迟绑定时间
	float BindOverlapTime = 0.25f;
	/**
	 * 延迟绑定计时器结束回调
	 */
	void BindOverlapTimerFinished();
public:	

};
