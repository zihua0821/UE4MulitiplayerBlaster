#include "HitScanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//获取拥有者
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
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//起始位置
		FVector Start = SocketTransform.GetLocation();
		//检测结果
		FHitResult FireHit;
		//进行射线检测
		WeaponTraceHit(Start, HitTarget, FireHit);
		//获取受击者
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();//不使用服务器倒带或者是本地控制的玩家
			if (HasAuthority() && bCauseAuthDamage)//服务器权威
			{
				//是否爆头
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				//应用伤害
				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bUseServerSideRewind)//非权威但使用服务器倒带
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
				{
					//请求得分
					BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
						);
				}
			}
		}
		//播放子弹爆炸效果
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		//命中音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		//枪口火焰
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		//开火音效
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}



void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		//检测终点
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		//执行射线检测
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else//没有命中则使用终点
		{
			OutHit.ImpactPoint = End;
		}
		//生成烟雾轨迹
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					TraceStart,
					FRotator::ZeroRotator,
					true
				);
			if (Beam)
			{
				//设置粒子方向
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
