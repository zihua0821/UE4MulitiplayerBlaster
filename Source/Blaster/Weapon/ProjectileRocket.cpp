#include "ProjectileRocket.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RocketMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

AProjectileRocket::AProjectileRocket()
{
	//初始化Mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//初始化移动组件
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::Destroyed()
{
	
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	//不碰撞自己
	if (OtherActor == GetOwner())
	{
		return;
	}
	//应用爆炸伤害
	ExplodeDamage();
	//开始摧毁计时器
	StartDestroyTimer();
	//生成爆炸粒子
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	//播放爆炸音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	//隐藏Mesh
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	//关闭碰撞
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//关闭粒子系统
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	//停止音效
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	//客户端绑定碰撞事件
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	//生成尾迹粒子
	SpawnTrailSystem();
	//播放火箭飞行音效
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

