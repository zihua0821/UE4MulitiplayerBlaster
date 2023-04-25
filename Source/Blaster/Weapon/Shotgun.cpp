#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	//调用基类开火
	AWeapon::Fire(FVector());
	//过去拥有者
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	//获取攻击者控制器
	AController* InstigatorController = OwnerPawn->GetController();
	//获取枪口插槽
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		//获取枪口变换
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//获取起始位置
		const FVector Start = SocketTransform.GetLocation();
		//击中结果Map
		TMap<ABlasterCharacter*, uint32> HitMap;
		//爆头结果Map
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
		//目标数组中全部执行射线检测
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			//本次目标结果
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			//受击者
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				//是否爆头
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				//计算总伤害
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter)) HeadShotHitMap[BlasterCharacter]++;
					else HeadShotHitMap.Emplace(BlasterCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter]++;
					else HitMap.Emplace(BlasterCharacter, 1);
				}
				//播放爆炸效果
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				//播放命中音效
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}
		}
		//受击的玩家数组
		TArray<ABlasterCharacter*> HitCharacters;
		//受击玩家伤害Map
		TMap<ABlasterCharacter*, float> DamageMap;
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				//计算伤害
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				//计算爆头伤害
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}
		//从Map中应用伤害
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, //受击角色
						DamagePair.Value, //总伤害
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
		if (!HasAuthority() && bUseServerSideRewind)//客户端且使用服务器倒带
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				//得分请求
				BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	//枪口插槽
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	//枪口变换
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	//起始位置
	const FVector TraceStart = SocketTransform.GetLocation();
	//归一化的指向位置
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	//偏移球心
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	//对每个弹丸执行射线检测
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		//随机单位向量
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		//随机点
		const FVector EndLoc = SphereCenter + RandVec;
		//目标向量
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
