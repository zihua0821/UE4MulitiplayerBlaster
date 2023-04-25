#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

/**
 * 武器枚举
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),//步枪
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),//火箭发射器
	EWT_Pistol UMETA(DisplayName = "Pistol"),//手枪
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),//冲锋枪
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),//霰弹枪
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),//狙击步枪
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),//榴弹发射器
	
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};