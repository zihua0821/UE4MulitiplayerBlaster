#include "Pickup.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickup::APickup()
{
	//允许Tick
	PrimaryActorTick.bCanEverTick = true;
	//允许复制
	bReplicates = true;
	//初始化根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//初始化球体组件
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);//附加到根组件
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//只查询
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//忽略对其他通道的碰撞
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//仅相应Pawn
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	//初始化Mesh组件
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);//附加到根组件
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//无碰撞
	PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));//缩放
	PickupMesh->SetRenderCustomDepth(true);//开始用户深度显示轮廓
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);//轮廓颜色

	//初始化粒子系统
	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);//附加到根组件
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	//在权威服务器上进行延迟绑定
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
					BindOverlapTimer,
					this,
					&APickup::BindOverlapTimerFinished,
					BindOverlapTime
				);
	}
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//旋转
	if (PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	//播放音效
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
	//播放粒子效果
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void APickup::BindOverlapTimerFinished()
{
	//延迟绑定
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

