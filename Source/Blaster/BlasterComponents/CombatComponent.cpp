#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"

UCombatComponent::UCombatComponent()
{
	//允许Tick
	PrimaryComponentTick.bCanEverTick = true;

}


void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	//拾取弹药
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		//拾取弹药不超过最大弹药
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	//如果拾取弹药时武器没有弹药，则自动换弹
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character)
	{
		//游戏开始时设置角色移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		//游戏开始时设置默认FOV
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		//如果是服务器权威版本则初始化携带的弹药
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//仅当装备武器时可以瞄准
	if (EquippedWeapon != nullptr)
	{
		//本地设置Aiming
		bAiming = bIsAiming;
		//调用ServerRPC将设置服务器的Aiming
		ServerSetAiming(bIsAiming);
		if (Character)
		{
			//本地更新瞄准移动速度
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
			//如果是狙击步枪则显示狙击镜
			if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
			{
				Character->ShowSniperScopeWidget(bIsAiming);
			}
		}
	}
	//如果是本地控制版本则设置瞄准按钮是否按下
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	//服务器权威版本设置Aiming并且设置瞄准移动速度
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	//当装备的武器被复制到客户端版本时客户端的行为
	if (EquippedWeapon && Character)
	{
		//设置武器状态
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		//将武器附加在右手上
		AttachActorToRightHand(EquippedWeapon);
		//启用控制器控制Yaw
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		//播放装备武器音效
		PlayEquipWeaponSound(EquippedWeapon);
		//禁用深度来禁止武器描边效果
		EquippedWeapon->EnableCustomDepth(false);
		//更新武器弹药HUD
		EquippedWeapon->SetHUDAmmo();
	}
	
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	//当副武器被复制到客户端版本时客户端的行为
	if (SecondaryWeapon && Character)
	{
		//设置武器状态
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		//将武器附加在玩家背后
		AttachActorToBackpack(SecondaryWeapon);
		//播放装备武器音效
		PlayEquipWeaponSound(EquippedWeapon);
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	//验证开火延迟，如果开火延迟不近似与武器开火延迟，则踢出玩家
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	//验证开火延迟，如果开火延迟不近似与武器开火延迟，则踢出玩家
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	//仅在装备武器时可以开火
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed && EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	//服务器权威版本时可以更新霰弹枪弹药
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	//播放换弹结束动画蒙太奇
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	//投掷手榴弹结束时恢复玩家状态并且将武器附加到右手
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	//隐藏手中的手榴弹
	ShowAttachedGrenade(false);
	//如果是本地玩家则调用投掷手榴弹RPC
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	//服务器权威版本投掷手榴弹
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		//获取手中手榴弹的位置
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		//计算手榴弹投出方向
		FVector ToTarget = Target - StartingLocation;
		//设置生成物的归属与伤害来源设置
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		//在世界中生成手榴弹
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//从携带的弹药中减去
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	//更新携带弹药HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	//调用换弹RPC为武器增加一个弹药
	ServerReloadAmmo(1);
	
}

void UCombatComponent::OnRep_Grenades()
{
	//当手榴弹数量被复制到客户端版本时客户端更新HUD
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	//更新手榴弹HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	//当同时装有主武器和副武器时可以交换武器
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		//一次开火后禁止开火
		bCanFire = false;
		if (EquippedWeapon)
		{
			//开火时准星的伸展量
			CrosshairShootingFactor = .75f;
			//跳转到不同武器的开火
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile://射弹武器
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan://射线检测武器
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun://霰弹枪
				FireShotgun();
				break;
			default:break;
			}
		}
		//启动开火计时器，计时器结束可以继续开火
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		//是否启用模拟散射
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		//客户端预测开火
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		//调用副武器RPC进行实际的开火
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		//是否启用模拟散射
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		//客户端预测开火
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		//调用副武器RPC进行实际的开火
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		//霰弹枪默认为启用模拟散射
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		//客户端预测开火
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		//调用副武器RPC进行实际的开火
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		//本地播放开火蒙太奇
		Character->PlayFireMontage(bAiming);
		//武器开火
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		//禁用本地换弹
		bLocallyReloading = false;
		//本地播放开火蒙太奇
		Character->PlayFireMontage(bAiming);
		//武器开火
		Shotgun->FireShotgun(TraceHitTargets);
		//开火时中断换弹
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;//为装备武器不能开火
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;//霰弹枪换弹时可以开火
	if (bLocallyReloading) return false;//本地正在换弹时不能开火
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;//子弹不为空，开火非冷却中且不在换弹时可以开火
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	//携带的弹药被复制到客户端版本时客户端更新HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	//处理霰弹枪中断换弹
	bool bJumpToShotgunEnd = 
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::OnRep_CombatState()
{
	//当战斗状态被复制到客户端版本时客户端的行为
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading://转换为换弹状态时
		//非本地控制的玩家处理换弹
		if (Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied://转换为闲置状态时
		if (bFireButtonPressed)//如果开火按键持续按下则开火
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade://转换为投掷手榴弹状态时
		if (Character && !Character->IsLocallyControlled())
		{
			//非本地控制的玩家处理投掷手榴弹
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons://转换为交换武器状态时
		if (Character && !Character->IsLocallyControlled())
		{
			//非本地控制的玩家处理交换武器
			Character->PlaySwapMontage();
		}
		break;
	default:break;
	}
}

void UCombatComponent::HandleReload()
{
	//播放换弹动画蒙太奇
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	//计算可以添加到武器的弹药数量
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//计算可添加子弹数
	int32 ReloadAmount = AmountToReload();
	//从携带的弹药中减去
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	//更新携带弹药HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	// EquippedWeapon->AddAmmo(ReloadAmount);
	//调用ServerRPC为武器增加子弹
	ServerReloadAmmo(ReloadAmount);
}


void UCombatComponent::ServerReloadAmmo_Implementation(int32 AmmoAmount)
{
	//多播通知每个对等体为其增加弹药
	MulticastReloadAmmo(AmmoAmount);
}

void UCombatComponent::MulticastReloadAmmo_Implementation(int32 AmmoAmount)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	//添加弹药
	EquippedWeapon->AddAmmo(AmmoAmount);
	//添加完成后可开火
	bCanFire = true;
	//如果霰弹枪子弹已满或者玩家携带的弹药为0则跳转到换弹结束
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	//多播通知霰弹枪开火
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	//服务器权威版本可开火
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	//计算准星位置
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	//计算准星的世界位置与方向
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (bScreenToWorld)
	{
		//射线开始位置
		FVector Start = CrosshairWorldPosition;
		//将开始位置向前移动，防止检测到自己
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		//射线结束位置
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		//执行射线检测
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);
		//如果没有结果则设置为检测终点
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else//将结果位置设为检测结果位置
		{
			HitTarget = TraceHitResult.ImpactPoint;
		}
		//通过判断检测到的Actor是否实现了接口来判断是否为玩家角色
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			//检测到玩家时将准星设为红色
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
	

}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	//仅当玩家处于空闲状态且装备有武器是可以投掷手榴弹
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	//切换到投掷状态
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		//播放投掷动画蒙太奇
		Character->PlayThrowGrenadeMontage();
		//将武器附加在左手
		AttachActorToLeftHand(EquippedWeapon);
		//显示手中的手榴弹
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		//非服务器权威玩家调用ServerRPC来投掷手榴弹
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		//服务器权威玩家更新手榴弹HUD
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	//掉落武器
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	//获取右手武器插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	//将武器附加到插槽
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	//是否使用手枪的特殊插槽
	bool bUsePistolSocket = 
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	//获取左手武器插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	//将武器附加到插槽
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	//获取背后武器插槽
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	//将武器附加到插槽
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	//选择当前武器的子弹
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	//更新子弹数
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		//在玩家的位置播放音效
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	//如果武器为空则换弹
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	//显示玩家角色手中的手榴弹
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//掉落装备中的武器
	DropEquippedWeapon();
	//装备当前武器
	EquippedWeapon = WeaponToEquip;
	//设置武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//将武器附加在右手
	AttachActorToRightHand(EquippedWeapon);
	//设置装备武器的拥有者
	EquippedWeapon->SetOwner(Character);
	//更新弹药
	EquippedWeapon->SetHUDAmmo();
	//更新玩家携带的对应弹药
	UpdateCarriedAmmo();
	//播放装备音效
	PlayEquipWeaponSound(WeaponToEquip);
	//为空子弹的武器换弹
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//装备服务器
	SecondaryWeapon = WeaponToEquip;
	//设置武器状态
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	//将副武器附加在背后
	AttachActorToBackpack(WeaponToEquip);
	//播放装备音效
	PlayEquipWeaponSound(WeaponToEquip);
	//设置装备武器的拥有者
	SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::OnRep_Aiming()
{
	//当瞄准状态复制到客户端版本时客户端，如果是本地控制的玩家，则设置瞄准状态
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	//服务器权威转换玩家状态
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		//播放动画蒙太奇
		Character->PlayThrowGrenadeMontage();
		//将武器附加在右手
		AttachActorToLeftHand(EquippedWeapon);
		//显示玩家手中的手榴弹
		ShowAttachedGrenade(true);
	}
	//更新手榴弹数量
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}


void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	//开启开火计时器
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	//计时器结束时可以开火
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	//弹药空时换弹
	ReloadEmptyWeapon();
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	//根据瞄准状态每帧对FOV进行插值
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	//设置FOV
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)
	{
		return;
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			//装备武器则显示准星
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//根据玩家移动速度来设置准星伸展量
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			HUDPackage.CrosshairSpread = 
				0.5f + 
				CrosshairVelocityFactor + 
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;
			//设置准星
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//本地非权威玩家退出
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	//多播开火
	MulticastFire(TraceHitTarget);
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//注册辅助变量
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);//仅拥有者复制
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	//非闲置状态不可装备武器
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)//只装备有主武器时装备副武器
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else//其他情况装备主武器
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	//装备武器后改变控制方式
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;
	//播放动画蒙太奇
	Character->PlaySwapMontage();
	//转换状态
	CombatState = ECombatState::ECS_SwappingWeapons;
	//设置没有交换完成
	Character->bFinishedSwapping = false;
	//禁用副武器的轮廓
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::Reload()
{
	//如果可以换弹则换弹
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		//调用ServerRPC换弹
		ServerReload();
		//播放换弹动画
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr)
	{
		return;
	}
	//设置换弹结束
	bLocallyReloading = false;
	//如果时是权威版本则设置状态更新弹药
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	//换弹时如果开火则等到换弹结束后立即开火
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::FinishSwap()
{
	//如果是权威版本则恢复玩家状态
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	//设置角色武器交换完成
	if (Character) Character->bFinishedSwapping = true;
	//显示副武器轮廓
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	//交换武器指针
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	//设置主武器状态
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//将主武器附加在右手
	AttachActorToRightHand(EquippedWeapon);
	//更新主武器弹药
	EquippedWeapon->SetHUDAmmo();
	//更新携带的弹药
	UpdateCarriedAmmo();
	//播放交换武器音效
	PlayEquipWeaponSound(EquippedWeapon);
	//设置副武器状态
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	//将副武器附加到背后
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	//如果武器弹药满则不进行换弹
	if (EquippedWeapon && EquippedWeapon->IsFull())
	{
		return;
	}
	//设置玩家换弹状态
	CombatState = ECombatState::ECS_Reloading;
	//如果非本地控制玩家则仅播放换弹动画
	if (!Character->IsLocallyControlled()) HandleReload();
}
