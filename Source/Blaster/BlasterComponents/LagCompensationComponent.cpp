#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Blaster.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	//启用组件Tick
	PrimaryComponentTick.bCanEverTick = true;
	
}


void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	//验证命中信息
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	//为每个被击中的角色应用伤害
	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		//计算爆头总伤害
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		//计算击中身体的伤害
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		//应用伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			HitCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//每帧记录帧数据
	SaveFramePackage();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	//获取帧数据
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	//验证命中
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//获取帧数据
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	//验证命中
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	//获取验证结果
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	//对结果应用伤害
	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)//如果链表节点数小于等于1
	{
		//储存当前帧数据
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		//将当前帧数据添加到链表头部
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)//如果记录的长度大于最大回溯时间
		{
			//移除头部的节点
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			//重新计算链表长度
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		//储存当前帧数据
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		//将当前帧数据添加到链表头部
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;//链表为空时返回
	if (bReturn) return FFramePackage();
	//验证的帧数据
	FFramePackage FrameToCheck;
	//是否需要插值
	bool bShouldInterpolate = true;
	//获取链表的头尾
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;//链表尾
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;//链表头
	if (OldestHistoryTime > HitTime)
	{
		//回溯记录太远，返回空数据
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)//命中时间正好命中
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)//命中时间在较近数据之前
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}
	//获取命中时间前后最近的两个帧数据
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();//较近的帧数据
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;//较远的帧数据
	while (Older->GetValue().Time > HitTime) //较远的数据依然比击中时间远
		{
		//OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
		}
	if (Older->GetValue().Time == HitTime) //命中时间正好命中，不需要插值
		{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
		}
	if (bShouldInterpolate)
	{
		//两个帧数据之间进行插值
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	//需要验证的帧数据数组
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	//霰弹枪验证命中
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//射弹武器验证结果
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	//根据验证结果引用伤害
	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
                                                                            const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	//霰弹枪命中结果
	FShotgunServerSideRewindResult ShotgunResult;
	//当前的帧数据数组
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		//当前的帧数据
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		//缓存当前帧数据
		CacheBoxPositions(Frame.Character, CurrentFrame);
		//将盒子移动到位置
		MoveBoxes(Frame.Character, Frame);
		//应用角色碰撞
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		//为爆头应用碰撞
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();
	//检查是否爆头
	for (auto& HitLocation : HitLocations)
	{
		//射线检测是否命中头部
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))//命中头部
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else//否则在Map中初始化记录
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	//检测时为所有盒子应用碰撞，结束时禁用碰撞
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//检查命中身体
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))//命中身体
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else//否则在Map中注册记录
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}
	//结束检测
	for (auto& Frame : CurrentFrames)
	{
		//重设盒子
		ResetHitBoxes(Frame.Character, Frame);
		//启用角色的碰撞
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	return ShotgunResult;
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		//记录时间
		Package.Time = GetWorld()->GetTimeSeconds();
		//记录角色
		Package.Character = Character;
		//记录盒子信息
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	//两帧间距离
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	//插值的分数
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	//储存插值结果
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	//插值每个盒子
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];
		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	//当前帧数据
	FFramePackage CurrentFrame;
	//缓存盒子位置
	CacheBoxPositions(HitCharacter, CurrentFrame);
	//移动盒子
	MoveBoxes(HitCharacter, Package);
	//禁用角色碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//启用头部盒子碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	//命中结果
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		//射线检测
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit) //击中头部
		{
			//重置盒子
			ResetHitBoxes(HitCharacter, CurrentFrame);
			//重新启用玩家碰撞
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else //没有命中头部，继续检测其他盒子
		{
			//启用每个盒子的碰撞
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			//射线检测
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			//命中返回结果
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}
	//重置盒子
	ResetHitBoxes(HitCharacter, CurrentFrame);
	//重新启用玩家角色碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//当前帧数据
	FFramePackage CurrentFrame;
	//缓存盒子位置
	CacheBoxPositions(HitCharacter, CurrentFrame);
	//移动盒子
	MoveBoxes(HitCharacter, Package);
	//禁用角色碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//启用头部盒子碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	//预测射弹轨迹信息
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;//是否开启碰撞检测
	PathParams.MaxSimTime = MaxRecordTime;//最大模拟时间
	PathParams.LaunchVelocity = InitialVelocity;//发射速度
	PathParams.StartLocation = TraceStart;//发射位置
	PathParams.SimFrequency = 15.f;//模拟频率
	PathParams.ProjectileRadius = 5.f;//射弹半径
	PathParams.TraceChannel = ECC_HitBox;//检测频道
	PathParams.ActorsToIgnore.Add(GetOwner());//忽略的Actor
	//预测射弹轨迹结果
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	if (PathResult.HitResult.bBlockingHit) //击中头部
	{
		//重置盒子
		ResetHitBoxes(HitCharacter, CurrentFrame);
		//重新启用玩家角色碰撞
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true, true };
	}
	else //没有命中头部，继续检测其他盒子
	{
		//启用每个盒子的碰撞
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		//射弹轨迹预测
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			//重置盒子
			ResetHitBoxes(HitCharacter, CurrentFrame);
			//重新启用玩家角色碰撞
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false };
		}
	}
	//重置盒子
	ResetHitBoxes(HitCharacter, CurrentFrame);
	//重新启用玩家角色碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	//读取盒子信息并且缓存
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	//将盒子移动到指定位置
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FVector());
			if (Package.HitBoxInfo.Contains(HitBoxPair.Key))
			{
				HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
				HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
				HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			}
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

