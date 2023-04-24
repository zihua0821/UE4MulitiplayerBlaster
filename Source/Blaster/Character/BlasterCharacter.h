#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class ABlasterPlayerState;
class ABlasterGameMode;
class ABlasterPlayerController;
class UCombatComponent;
class UBuffComponent;
class ULagCompensationComponent;
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UBoxComponent;
class UWidgetComponent;
class USoundCue;

//玩家离开游戏代理，动态多播
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

/**
 * 玩家角色
 * 处理玩家移动 按键事件 动画蒙太奇等
 */
UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	//Tick
	virtual void Tick(float DeltaTime) override;
	//处理绑定输入
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//注册复制变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//初始化组件
	virtual void PostInitializeComponents() override;

	/**
	 * 播放开火动画蒙太奇
	 * @param bAiming 是否在瞄准
	 */
	void PlayFireMontage(bool bAiming);
	
	/**
	 * 播放淘汰动画蒙太奇
	 */
	void PlayElimMontage();

	/**
	 * 播放被击中动画蒙太奇
	 */
	void PlayHitReactMontage();

	/**
	 * 播放换弹动画蒙太奇
	 */
	void PlayReloadMontage();

	/**
	 * 播放投掷手榴弹动画蒙太奇
	 */
	void PlayThrowGrenadeMontage();

	/**
	 * 播放交换武器动画蒙太奇
	 */
	void PlaySwapMontage();

	/**
	 * 淘汰
	 * @param bPlayerLeftGame 是否为退出游戏
	 */
	void Elim(bool bPlayerLeftGame);

	/**
	 * 淘汰
	 * 多播
	 * @param bPlayerLeftGame 是否为退出游戏
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	/**
	 * 角色被摧毁时
	 */
	virtual void Destroyed() override;

	//是否启用游戏控制 复制变量
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	/**
	 * 显示狙击镜
	 * @param bShowScope 是否显示狙击镜
	 * 蓝图重写函数
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	/**
	 * 更新生命值HUD
	 */
	void UpdateHUDHealth();

	/**
	 * 更新护盾值HUD
	 */
	void UpdateHUDShield();

	/**
	 * 更新弹药HUD
	 */
	void UpdateHUDAmmo();

	/**
	 * 生成默认武器
	 */
	void SpawnDefaultWeapon();

	/**
	 * 受击盒指针Map
	 */
	UPROPERTY()
	TMap<FName,UBoxComponent*> HitCollisionBoxes;

	//是否交换武器完成
	bool bFinishedSwapping = false;

	/**
	 * 离开游戏
	 * ServerRPC
	 */
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	//声明离开游戏代理
	FOnLeftGame OnLeftGame;

	/**
	 * 获取皇冠
	 * 多播
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	/**
	 * 失去皇冠
	 * 多播
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	/**
	 * 设置队伍颜色
	 * @param Team 队伍
	 */
	void SetTeamColor(ETeam Team);
	
protected:
	
	virtual void BeginPlay() override;

	/**
	 * 向前后移动
	 * @param Value 移动的键值
	 */
	void MoveForward(float Value);
	/**
	 * 向左右移动
	 * @param Value 移动的键值
	 */
	void MoveRight(float Value);
	
	/**
	 * 鼠标左右旋转
	 * @param Value 移动的键值
	 */
	void Turn(float Value);
	/**
	 * 鼠标上下旋转
	 * @param Value 移动的键值
	 */
	void LookUp(float Value);
	
	/**
	 * 装备武器按键按下事件
	 */
	void EquipButtonPressed();
	/**
	 * 蹲伏按键按下事件
	 */
	void CrouchButtonPressed();
	/**
	 * 蹲伏按键松开事件
	 */
	void CrouchButtonReleased();
	/**
	 * 瞄准按键按下事件
	 */
	void AimButtonPressed();
	/**
	 * 瞄准按键松开事件
	 */
	void AimButtonReleased();
	/**
	 * 换弹按键按下事件
	 */
	void ReloadButtonPressed();

	/**
	 * 瞄准偏移
	 * @param DeltaTime 增量事件
	 */
	void AimOffset(float DeltaTime);

	/**
	 * 跳跃
	 * 引擎内置
	 */
	virtual void Jump() override;

	/**
	 * 开火按键按下事件
	 */
	void FireButtonPressed();

	/**
	 * 开火按键松开事件
	 */
	void FireButtonReleased();

	/**
	 * 玩家受伤
	 * @param DamagedActor 被攻击者Actor
	 * @param Damage 伤害值
	 * @param DamageType 伤害类型
	 * @param InstigatorController 攻击者控制器
	 * @param DamageCauser 伤害指针
	 */
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	/**
	 * 轮询初始化
	 */
	void PollInit();
	
	/**
	 * 原地转向
	 * @param DeltaTime 增量事件
	 */
	void RotateInPlace(float DeltaTime);

	/**
	 * 投掷手榴弹按下事件
	 */
	void GrenadeButtonPressed();

	/**
	 * 掉落或者销毁武器
	 * @param Weapon 武器指针
	 */
	void DropOrDestroyWeapon(AWeapon* Weapon);

	/**
	 * 掉落或者销毁武器
	 */
	void DropOrDestroyWeapons();

	/**
	 * 设置重生点
	 */
	void SetSpawnPoint();

	/**
	 * 当玩家状态初始化完成时
	 */
	void OnPlayerStateInitialized();

	/**
	 * 受击盒
	 */
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
private:

	//弹簧臂组件
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;
	//相机组件
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;
	
	//头顶部件组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;
	
	//与玩家重叠的武器 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	/**
	 * 当与玩家重叠的武器被复制更新时的回调
	 */
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	//战斗组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	//BUFF组件
	UPROPERTY(VisibleAnywhere)
	UBuffComponent* Buff;

	//延迟补偿组件
	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

	/**
	 * 装备武器按下事件
	 * ServerRPC
	 */
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	//瞄准便宜的Yaw与Pitch
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	//起始旋转
	FRotator StartingAimRotation;

	/**
	 * 原地转向枚举
	 */
	ETurningInPlace TurningInPlace;
	/**
	 * 原地转向
	 * @param DeltaTime 增量事件
	 */
	void TurnInPlace(float DeltaTime);

	//开火动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;
	//受击动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	//淘汰动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;
	//换弹动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	//投掷手榴弹动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	//交换武器动画蒙太奇
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	/**
	 * 相机离玩家过近时隐藏玩家Mesh
	 */
	void HideCameraIfCharacterClose();
	//隐藏阈值
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	//玩家最大生命值
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	//玩家生命值 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	/**
	 * 玩家生命值复制更新时触发的回调
	 * @param LastHealth 上一次的生命值
	 */
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	//玩家最大护盾值
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;
	//玩家护盾 复制变量 复制更新时触发回调
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 100.f;
	/**
	 * 玩家护盾值复制更新时触发的回调
	 * @param LastShield 上一次的护盾值
	 */
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	//玩家控制器指针
	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	//玩家是否被淘汰
	bool bElimmed = false;
	//淘汰计时器
	FTimerHandle ElimTimer;
	//淘汰重生延迟
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	/**
	 * 淘汰计时器结束是触发的回调
	 */
	void ElimTimerFinished();

	//玩家是否离开游戏
	bool bLeftGame = false;

	//角色溶解的时间线
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	//角色溶解轨道
	FOnTimelineFloat DissolveTrack;
	//角色溶解曲线
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	/**
	 * 更新角色溶解材质
	 * @param DissolveValue 溶解值
	 */
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	/**
	 * 开始溶解
	 */
	void StartDissolve();
	//动态溶解材质实例
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//溶解材质实例
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;
	//红色溶解材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;
	//红色材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;
	//蓝色溶解材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;
	//蓝色材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;
	//橘色材质实例
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* OriginalMaterial;
	
	//玩家淘汰粒子系统
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	//玩家淘汰粒子系统组件
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	//玩家淘汰音效
	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	//玩家状态
	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;
	
	//皇冠粒子系统
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;
	//皇冠粒子系统组件
	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	//手榴弹网格体组件
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;
	
	//默认武器子类
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//游戏模式指针
	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;
	
public:

	/**
	 * 设置重合的武器
	 * @param Weapon 武器指针
	 */
	void SetOverlappingWeapon(AWeapon* Weapon);
	/**
	 * @return 是否装备武器
	 */
	bool IsWeaponEquipped();
	/**
	 * @return 是否正在瞄准
	 */
	bool IsAiming();
	/**
	 * @return 获取AO_Yaw
	 */
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw;}
	/**
	 * @return 获取AO_Pitch
	 */
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch;}
	/**
	 * @return 获取装备的武器
	 */
	AWeapon* GetEquippedWeapon();
	/**
	 * @return 获取旋转枚举
	 */
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace;}
	/**
	 * @return 获取攻击目标
	 */
	FVector GetHitTarget() const;
	/**
	 * @return 获取相机组件指针
	 */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;}
	/**
	 * @return 是否被淘汰
	 */
	FORCEINLINE bool IsElimmed() const { return bElimmed;}
	/**
	 * @return 获取生命值
	 */
	FORCEINLINE float GetHealth() const { return Health;}
	/**
	 * 设置生命值
	 * @param Amount 生命值
	 */
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	/**
	 * @return 获取最大生命值
	 */
	FORCEINLINE float GetMaxHealth() const { return MaxHealth;}
	/**
	 * @return 获取护盾值
	 */
	FORCEINLINE float GetShield() const { return Shield; }
	/**
	 * 设置护盾值
	 * @param Amount 最大护盾值
	 */
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	/**
	 * @return 获取最大护盾值
	 */
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	/**
	 * @return 获取战斗状态
	 */
	ECombatState GetCombatState() const;
	/**
	 * @return 获取延战斗组件指针
	 */
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	/**
	 * @return 是否可以进行游戏
	 */
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	/**
	 * @return 获取换弹动画蒙太奇指针
	 */
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	/**
	 * @return 获取手榴弹静态网格体组件指针
	 */
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	/**
	 * @return 获取BUFF组件指针
	 */
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	/**
	 * @return 是否本地换弹
	 */
	FORCEINLINE bool IsLocallyReloading() const;
	/**
	 * @return 获取延迟补偿组件指针
	 */
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};


