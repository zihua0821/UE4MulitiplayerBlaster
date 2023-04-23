#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;
class AProjectile;
class ABlasterPlayerController;

/**
 * 战斗组件
 * 处理战斗相关逻辑
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;

	//组件Tick
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 装备武器
	 * @param WeaponToEquip 装备的武器指针
	 */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/**
	 * 交换装备的武器
	 */
	void SwapWeapons();

	/**
	 * 换弹
	 */
	void Reload();

	/**
	 * 换弹结束通知回调
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/**
	 * 交换武器结束通知回调
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	/**
	 * 交换武器更改附加通知回调
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	/**
	 * 设置准星
	 * @param DeltaTime 增量时间
	 */
	void SetHUDCrosshairs(float DeltaTime);

	/**
	 * 对FOV插值
	 * @param DeltaTime 增量时间
	 */
	void InterpFOV(float DeltaTime);

	/**
	 * 开火按键事件
	 * @param bPressed 是否按下
	 */
	void FireButtonPressed(bool bPressed);

	/**
	 * 霰弹枪每次换弹完成通知回调
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	/**
	 * 将动画蒙太奇跳至换弹结束
	 */
	void JumpToShotgunEnd();

	/**
	 * 投掷手榴弹完成通知回调
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	/**
	 * 投掷手榴弹
	 * 蓝图调用
	 */
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	/**
	 * 在服务器发射榴弹
	 * ServerRPC
	 * @param Target 目标向量
	 */
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	/**
	 * 拾取弹药
	 * @param WeaponType 武器的类型枚举
	 * @param AmmoAmount 拾取的弹药数量
	 */
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	//是否使用本地换弹（客户端预测）
	bool bLocallyReloading = false;
	
protected:

	//游戏开始时
	virtual void BeginPlay() override;

	/**
	 * 设置是否瞄准
	 * @param bIsAiming 是否瞄准
	 */
	void SetAiming(bool bIsAiming);

	/**
	 * 设置是否瞄准
	 * ServerRPC
	 * @param bIsAiming 是否瞄准
	 */
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	/**
	 * 装备武器复制更新回调
	 */
	UFUNCTION()
	void OnRep_EquippedWeapon();

	/**
	 * 装备副武器复制更新回调
	 */
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	/**
	 * 开火
	 * ServerRPC
	 * 带有校验
	 * @param TraceHitTarget 射线检测得到的目标向量
	 * @param FireDelay 开火延迟
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	/**
	 * 开火
	 * Server多播
	 * @param TraceHitTarget 射线检测得到的目标向量
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/**
	 * 换弹
	 * ServerRPC
	 * @param AmmoAmount 换弹数量
	 */
	UFUNCTION(Server, Reliable)
	void ServerReloadAmmo(int32 AmmoAmount);

	/**
	 * 换弹
	 * Server多播
	 * @param AmmoAmount 换弹数量
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReloadAmmo(int32 AmmoAmount);

	/**
	 * 霰弹枪开火
	 * ServerRPC
	 * 带有校验
	 * @param TraceHitTargets 射线检测得到的目标向量数组
	 * @param FireDelay 开火延迟
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	/**
	 * 霰弹枪开火
	 * Server多播
	 * @param TraceHitTargets 射线检测得到的目标向量数组
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	/**
	 * 从准星位置执行射线检测
	 * @param TraceHitResult 射线检测结果引用
	 */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/**
	 * 换弹
	 * ServerRPC
	 */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	/**
	 * 投掷手榴弹
	 */
	void ThrowGrenade();

	/**
	 * 投掷手榴弹
	 * ServerRPC
	 */
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	//手榴弹的子类 蓝图中配置
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	/**
	 * 掉落装备的武器
	 */
	void DropEquippedWeapon();

	/**
	 * 将武器连接到右手
	 * @param ActorToAttach 连接的武器
	 */
	void AttachActorToRightHand(AActor* ActorToAttach);

	/**
	 * 将武器连接到左手
	 * @param ActorToAttach 连接的武器
	 */
	void AttachActorToLeftHand(AActor* ActorToAttach);

	/**
	 * 将武器连接到背后
	 * @param ActorToAttach 连接的武器
	 */
	void AttachActorToBackpack(AActor* ActorToAttach);

	/**
	 * 更新携带的弹药
	 */
	void UpdateCarriedAmmo();

	/**
	 * 播放装备武器音效
	 * @param WeaponToEquip 装备的武器
	 */
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	/**
	 * 为空弹药武器换弹
	 */
	void ReloadEmptyWeapon();

	/**
	 * 显示手中的手榴弹
	 * @param bShowGrenade 是否显示
	 */
	void ShowAttachedGrenade(bool bShowGrenade);

	/**
	 * 装备到主武器
	 * @param WeaponToEquip 武器指针
	 */
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	
	/**
	 * 装备到副武器
	 * @param WeaponToEquip 武器指针
	 */
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
	//玩家角色指针
	UPROPERTY()
	ABlasterCharacter* Character;
	
	//玩家控制器指针
	UPROPERTY()
	ABlasterPlayerController* Controller;

	//玩家HUD指针
	UPROPERTY()
	ABlasterHUD* HUD;

	//玩家主武器指针 复制变量 复制更新触发回调
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	//玩家副武器指针 复制变量 复制更新触发回调
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	//玩家是否瞄准 复制变量 复制更新触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	//玩家是否瞄准复制更新回调
	UFUNCTION()
	void OnRep_Aiming();
	
	//瞄准按键是否按下
	bool bAimButtonPressed = false;

	//玩家基本行走速度
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 600.f;

	//玩家瞄准行走速度
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 450.f;

	//开火按键是否按下
	bool bFireButtonPressed;

	//准星在奔跑、空中、瞄准、射击时的伸展量
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	//默认FOV
	float DefaultFOV;

	//开镜后缩放的FOV
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	//玩家当前的FOV
	float CurrentFOV;

	//FOV缩放插值速度
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	//准星信息
	FHUDPackage HUDPackage;

	//开火冷却计时器
	FTimerHandle FireTimer;

	//玩家是否可以开火
	bool bCanFire = true;

	/**
	 * 开火冷却计时器开始
	 */
	void StartFireTimer();

	/**
	 * 开火冷却计时器结束回调
	 */
	void FireTimerFinished();

	/**
	 * 玩家开火
	 */
	void Fire();

	/**
	 * 射弹武器开火
	 */
	void FireProjectileWeapon();

	/**
	 * 射线检测武器开火
	 */
	void FireHitScanWeapon();

	/**
	 * 霰弹枪开火
	 */
	void FireShotgun();

	/**
	 * 本地开火
	 * 用于客户端预测
	 * @param TraceHitTarget 射线检测得到的目标向量
	 */
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	/**
	 * 霰弹枪本地开火
	 * 用于客户端预测
	 * @param TraceHitTargets 射线检测得到的目标向量数组
	 */
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	/**
	 * 是否可以开火
	 * @return 是否可以开火
	 */
	bool CanFire();

	//玩家携带的弹药数 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	/**
	 * 玩家携带的弹药数复制更新的回调
	 */
	UFUNCTION()
	void OnRep_CarriedAmmo();

	/**
	 * 玩家携带弹药HashMap
	 * @Key EWeaponType 武器类型
	 * @Value int32 武器弹药数
	 */
	TMap<EWeaponType, int32> CarriedAmmoMap;

	//最大携带弹药数
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	//游戏开始携带步枪弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 0;

	//游戏开始携带火箭发射器弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	//游戏开始携带手枪弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	//游戏开始携带冲锋枪弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	//游戏开始携带霰弹枪弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	//游戏开始携带狙击步枪弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	//游戏开始携带榴弹发射器弹药数
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	//初始化携带弹药
	void InitializeCarriedAmmo();

	//玩家的战斗状态 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	/**
	 * 玩家的战斗状态复制更新时触发的回调
	 */
	UFUNCTION()
	void OnRep_CombatState();

	/**
	 * 处理换弹
	 */
	void HandleReload();

	/**
	 * @return 可以添加到武器的弹药数
	 */
	int32 AmountToReload();

	/**
	 * 更新武器弹药
	 */
	void UpdateAmmoValues();

	/**
	 * 更新霰弹枪弹药
	 */
	void UpdateShotgunAmmoValues();

	//手榴弹携带数量 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	/**
	 * 手榴弹携带数量复制更新时触发的回调
	 */
	UFUNCTION()
	void OnRep_Grenades();

	//手榴弹最大携带数量
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	/**
	 * 更新手榴弹数量
	 */
	void UpdateHUDGrenades();
	
public:

	//目标向量
	FVector HitTarget;
	
	/**
	 * 获取当前手榴弹数
	 * @return 当前手榴弹数
	 */
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	
	/**
	 * 获取是否可以交换武器
	 * @return 是否可以交换武器
	 */
	bool ShouldSwapWeapons();
};



