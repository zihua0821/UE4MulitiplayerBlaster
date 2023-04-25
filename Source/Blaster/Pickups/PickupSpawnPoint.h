#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

/**
 * 拾取物生成点
 */
UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	//拾取物子类数组
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickup>> PickupClasses;

	//拾取物指针
	UPROPERTY()
	APickup* SpawnedPickup;

	/**
	 * 生成拾取物
	 */
	void SpawnPickup();

	/**
	 * 生成拾取物计时器结束回调
	 */
	void SpawnPickupTimerFinished();

	/**
	 * 开启生成拾取物计时器
	 */
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	
private:

	//生成计时器
	FTimerHandle SpawnPickupTimer;

	//最小生成时间
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;
	
	//最大生成时间
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;
public:	

};
