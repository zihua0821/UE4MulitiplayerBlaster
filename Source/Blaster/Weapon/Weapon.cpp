#include "Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Casing.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

AWeapon::AWeapon()
{
	//允许Tick
	PrimaryActorTick.bCanEverTick = true;
	//允许复制
	bReplicates = true;
	SetReplicateMovement(true);
	
	//初始化网格体
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	//设置网格体碰撞
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//启用深度
	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	
	//初始化球体区域
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//初始化拾取控件
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	//设置球体区域碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	//绑定开始覆盖事件
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
	//绑定结束覆盖事件
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	//拾取控件不可见
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	default:break;
	}
}

void AWeapon::OnEquipped()
{
	//拾取控件不可见
	ShowPickupWidget(false);
	//禁用球体区域碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//禁用Mesh物理
	WeaponMesh->SetSimulatePhysics(false);
	//禁用Mesh重力
	WeaponMesh->SetEnableGravity(false);
	//禁用Mesh碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//启用冲锋枪飘带的物理
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	//关闭深度
	EnableCustomDepth(false);
	//绑定高Ping事件
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	//权威服务器开启碰撞
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	//开启物理
	WeaponMesh->SetSimulatePhysics(true);
	//开启重力
	WeaponMesh->SetEnableGravity(true);
	//启用碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//忽略对Pawn碰撞
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//忽略对相机碰撞
	//开启深度
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	//解除高Ping绑定
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	//拾取控件不可见
	ShowPickupWidget(false);
	//关闭球体区域碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//关闭物理
	WeaponMesh->SetSimulatePhysics(false);
	//关闭重力
	WeaponMesh->SetEnableGravity(false);
	//禁用碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//开启冲锋枪飘带物理
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	//设置深度
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
	//解除高Ping绑定
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//注册复制变量
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	//播放开火动画
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	//生成子弹壳
	if (CasingClass)
	{
		//获取弹壳弹出位置
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			//获取弹出变换
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				//生成弹壳
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	//花费子弹
	SpendRound();
}

void AWeapon::Dropped()
{
	//设置武器状态
	SetWeaponState(EWeaponState::EWS_Dropped);
	//解除附加规则
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	//解除武器附加
	WeaponMesh->DetachFromComponent(DetachRules);
	//重置武器归属
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AddToAmmo)
{
	//钳制弹药数量
	Ammo = FMath::Clamp(Ammo + AddToAmmo, 0, MagCapacity);
	//更新HUD
	SetHUDAmmo();
	//客户端增加弹药
	ClientAddAmmo(AddToAmmo);
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	//获取枪口插槽
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	//获取枪口变换
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	//获取起始位置
	const FVector TraceStart = SocketTransform.GetLocation();
	//归一化发射方向
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	//获取偏移球球心位置
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	//获取随机三维单位向量
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	//计算目标向量
	const FVector EndLoc = SphereCenter + RandVec;
	//计算偏移后的发射方向
	const FVector ToEndLoc = EndLoc - TraceStart;
	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(
	// 	GetWorld(),
	// 	TraceStart,
	// 	FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
	// 	FColor::Cyan,
	// 	true);
	
	//返回偏移后的目标位置
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//设置覆盖的武器
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//清空覆盖的武器
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}

}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	//如果没有拥有者则恢复默认
	if (GetOwner() == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else//否则设置
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			//更新武器弹药
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	//高Ping时使用服务器倒带
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	//武器状态变化时
	OnWeaponStateSet();
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	//服务器退出
	if (HasAuthority()) return;
	
	//对客户端预测进行序列验证
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;

	//更新武器弹药
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	//钳制武器弹药
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		//如果子弹满则跳到霰弹枪换弹结束
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	//更新武器弹药
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	//钳制花费的弹药
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	//更新武器弹药
	SetHUDAmmo();
	//如果是权威服务器则通知客户端更新
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else//否则序列增加
	{
		++Sequence;
	}
}



void AWeapon::SetWeaponState(EWeaponState State)
{
	//设置武器状态
	WeaponState = State;
	//武器状态设置后
	OnWeaponStateSet();
}
