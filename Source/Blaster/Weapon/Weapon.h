#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

class USoundCue;
class ABlasterCharacter;
class ABlasterPlayerController;
class USphereComponent;
class UWidgetComponent;
class ACasing;

/**
 * 武器状态枚举
 */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),//初始状态
	EWS_Equipped UMETA(DisplayName = "Equipped"),//装备中
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),//作为副武器装备中
	EWS_Dropped UMETA(DisplayName = "Dropped"),//掉落
	
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * 武器类别枚举
 */
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),//射线检查武器
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),//射弹武器
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),//霰弹枪

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 武器基类
 */
UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	//Tick
	virtual void Tick(float DeltaTime) override;
	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//重写当拥有者复制更新变化时的回调
	virtual void OnRep_Owner() override;
	
	/**
	 * 设置武器弹药HUD
	 */
	void SetHUDAmmo();
	/**
	 * 显示拾取控件
	 * @param bShowWidget 是否显示
	 */
	void ShowPickupWidget(bool bShowWidget);
	/**
	 * 武器开火
	 * @param HitTarget 攻击目标位置
	 */
	virtual void Fire(const FVector& HitTarget);
	/**
	 * 武器掉落
	 */
	void Dropped();
	/**
	 * 为武器增加弹药
	 * @param AddToAmmo 弹药数量
	 */
	void AddAmmo(int32 AddToAmmo);

	/**
	 * 使用散射偏移进行射线追踪
	 * @param HitTarget 弹药数量
	 * @return 结果位置
	 */
	FVector TraceEndWithScatter(const FVector& HitTarget);

	/**
	 * 准星材质
	 */
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	//缩放后的FOV
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	//FOV缩放插值速度
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	//开火延迟
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	//是否自动开火
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	//装备音效
	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	/**
	 * 启用用户深度
	 * @param bEnable 是否开启
	 */
	void EnableCustomDepth(bool bEnable);

	//是否摧毁武器
	bool bDestroyWeapon = false;

	//开火类型
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	//是否使用散射
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
protected:
	virtual void BeginPlay() override;

	/**
	 * 武器状态修改时
	 */
	virtual void OnWeaponStateSet();
	/**
	 * 武器装备时
	 */
	virtual void OnEquipped();
	/**
	 * 武器掉落时
	 */
	virtual void OnDropped();
	/**
	 * 副武器装备时
	 */
	virtual void OnEquippedSecondary();

	/**
	 * 当球体覆盖时
	 * @param OverlappedComponent 覆盖组件
	 * @param OtherActor 覆盖的Actor
	 * @param OtherComp 覆盖的组件
	 * @param OtherBodyIndex 覆盖的Actor编号
	 * @param bFromSweep 是否交换
	 * @param SweepResult 交换结果
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/**
	 * 当球体覆盖时
	 * @param OverlappedComponent 覆盖组件
	 * @param OtherActor 覆盖的Actor
	 * @param OtherComp 覆盖的组件
	 * @param OtherBodyIndex 覆盖的Actor编号
	 */
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	//散射到球体的距离
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	//散射球体的半径
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	//武器伤害
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//爆头伤害
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	//是否启用副武器倒带
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	//玩家角色指针
	UPROPERTY()
	ABlasterCharacter* BlasterOwnerCharacter;

	//玩家控制器指针
	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController;

	/**
	 * 高Ping时
	 * @param bPingTooHigh 是否高Ping
	 */
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
	
private:

	//武器静态网格体组件
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	//武器球体区域
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	//武器状态 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	//武器状态复制更新时触发的回调
	UFUNCTION()
	void OnRep_WeaponState();

	//拾取部件
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	//开火动画
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;

	//弹壳子类
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	//弹药数
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	/**
	 * 更新弹药
	 * ClientRPC
	 * @param ServerAmmo 弹药数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	/**
	 * 添加弹药
	 * ClientRPC
	 * @param AmmoToAdd 弹药数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	/**
	 * 花费一次弹药
	 */
	void SpendRound();

	//武器弹匣最大弹药
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	//添加弹药的序列 用于客户端预测
	int32 Sequence = 0;
		
	//武器类型
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:

	/**
	 * 设置武器状态
	 * @param State 武器状态
	 */
	void SetWeaponState(EWeaponState State);
	/**
	 * 获取球体区域组件指针
	 * @return 球体区域组件指针
	 */
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	/**
	 * 获取武器网格体
	 * @return 武器网格体
	 */
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	/**
	 * 获取缩放的FOV
	 * @return 缩放的FOV
	 */
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV;}
	/**
	 * 获取FOV插值速度
	 * @return FOV插值速度
	 */
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed;}
	/**
	 * 获取弹匣是否满
	 * @return 弹匣是否空
	 */
	FORCEINLINE bool IsEmpty() const { return Ammo <= 0;}
	/**
	 * 获取弹匣是否满
	 * @return 弹匣是否满
	 */
	FORCEINLINE bool IsFull() const { return Ammo == MagCapacity;}
	/**
	 * 获取武器类型
	 * @return 武器类型
	 */
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType;}
	/**
	 * 获取武器弹药
	 * @return 武器弹药
	 */
	FORCEINLINE int32 GetAmmo() const { return Ammo;}
	/**
	 * 设置武器弹药
	 * @param InAmmo 武器弹药
	 */
	FORCEINLINE void SetAmmo(int32 InAmmo)  { Ammo = InAmmo;}
	/**
	 * 获取弹匣弹药量
	 * @return 弹匣弹药量
	 */
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity;}
	/**
	 * 获取伤害
	 * @return 伤害
	 */
	FORCEINLINE float GetDamage() const { return Damage; }
	/**
	 * 获取爆头伤害
	 * @return 爆头伤害
	 */
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};


