#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	//禁止Tick
	PrimaryActorTick.bCanEverTick = false;
	//初始化Mesh
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	//设置弹出速度
	ShellEjectionImpulse = 10.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	//绑定碰撞事件
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//施加弹出速度
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	//与地面碰撞时播放碰撞音效
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	//摧毁
	Destroy();
}
