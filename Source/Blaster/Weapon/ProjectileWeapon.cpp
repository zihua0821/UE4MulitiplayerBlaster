#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//获取攻击者
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//获取枪口插槽
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		//获取枪口变换
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//从枪口到攻击目标
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//目标旋转
		FRotator TargetRotation = ToTarget.Rotation();
		//设置生成子弹参数
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		//生成的子弹指针
		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)//使用服务器倒带
		{
			if (InstigatorPawn->HasAuthority()) //服务器
			{
				if (InstigatorPawn->IsLocallyControlled()) //服务器本地控制的玩家
				{
					//生成子弹
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					//应用伤害
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
				else //服务器，不是本地控制的玩家
				{
					//生成服务器倒带子弹
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else //客户端
			{
				if (InstigatorPawn->IsLocallyControlled())//客户端本地控制的玩家
				{
					//生成服务器倒带子弹
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					//设置方向
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				}
				else //客户端，非本地控制玩家
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					//不使用服务倒带
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else//不使用服务器倒带
		{
			if (InstigatorPawn->HasAuthority())//服务器
			{
				//生成子弹
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				//应用伤害
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}
