#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;
class UBoxComponent;
class UProjectileMovementComponent;

/**
 * 子弹类
 * 用于射弹武器
 */
UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	
	//重写Tick
	virtual void Tick(float DeltaTime) override;
	//重写摧毁
	virtual void Destroyed() override;

	//是否使用服务器倒带
	bool bUseServerSideRewind = false;

	//发射起始位置
	FVector_NetQuantize TraceStart;
	//发射速度
	FVector_NetQuantize100 InitialVelocity;

	//子弹速度
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	//子弹伤害，仅为火箭弹与榴弹设置
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//爆头伤害，不要为火箭弹与榴弹设置
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;
	
protected:

	//重写开始
	virtual void BeginPlay() override;

	/**
	 * 开始销毁计时
	 */
	void StartDestroyTimer();

	/**
	 * 销毁计时器结束回调
	 */
	void DestroyTimerFinished();

	/**
	 * 生成轨迹粒子系统
	 */
	void SpawnTrailSystem();

	/**
	 * 应用爆炸伤害
	 */
	void ExplodeDamage();

	/**
	 * 碰撞时回调
	 * @param HitComp 碰撞组件
	 * @param OtherActor 碰撞的Actor
	 * @param OtherComp 碰撞的组件
	 * @param NormalImpulse 单位冲击力
	 * @param Hit 命中结果
	 */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	//爆炸粒子系统
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	//爆炸音效
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	//碰撞盒
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	//尾迹粒子系统
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	//尾迹粒子系统组件
	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	//子弹移动组件
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	//子弹Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	//爆炸内半径
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	//爆炸外半径
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:

	//摧毁计时器
	FTimerHandle DestroyTimer;

	//摧毁时间
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
	//轨迹粒子系统
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	//轨迹粒子系统组件
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

public:	

};
