#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	//初始化Mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//初始化移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::Destroyed()
{
	//销毁前应用爆炸伤害
	ExplodeDamage();
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	//生成尾迹
	SpawnTrailSystem();
	//开启销毁计时器
	StartDestroyTimer();
	//绑定弹跳事件
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	//播放弹跳音效
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}
