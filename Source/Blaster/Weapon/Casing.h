#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

class USoundCue;

/**
 * 弹壳类
 */
UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	//重写碰撞时
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
private:
	
	//弹壳Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	//弹壳弹出速度
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	//弹壳落地音效
	UPROPERTY(EditAnywhere)
	USoundCue* ShellSound;
public:	

};
