#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UCharacterOverlay;
class UAnnouncement;
class UElimAnnouncement;

//准星结构
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

	//中心与四个方向材质
	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	//准星伸展量
	float CrosshairSpread;
	//准星颜色
	FLinearColor CrosshairsColor;
};

/**
 * 游戏界面
 * 继承自HUD
 * 负责游戏界面渲染
 * 可以添加各个控件
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:

	//渲染HUD
	virtual void DrawHUD() override;

	//玩家覆盖控件的子类，蓝图中配置
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	/**
	 * 将玩家覆盖控件添加到界面
	 */
	void AddCharacterOverlay();

	//玩家覆盖控件指针
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	//公告控件的子类，蓝图中配置
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	//公告控件指针
	UPROPERTY()
	UAnnouncement* Announcement;

	/**
	 * 将公告控件添加到界面
	 */
	void AddAnnouncment();

	/**
	 * 将玩家淘汰公告控件添加到界面
	 * @param Attacker 击败者字符串
	 * @param Victim 被击败者字符串
	 */
	void AddElimAnnouncement(FString Attacker, FString Victim);
	
protected:

	//游戏开始时
	virtual void BeginPlay() override;
	
private:

	//当前玩家控制器指针
	UPROPERTY()
	APlayerController* OwningPlayer;

	//准星结构体
	FHUDPackage HUDPackage;

	/**
	 * 绘制准星
	 * @param Texture 材质指针
	 * @param ViewportCenter 视口中心向量
	 * @param Spread 准星伸展量
	 * @param CrosshairColor 准星颜色
	 */
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	//最大准星伸展量
	float CrosshairSpreadMax = 16.f;

	//玩家淘汰公告控件子类，蓝图中配置
	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	//玩家淘汰公告持续时间，蓝图中配置
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;

	/**
	 * 玩家淘汰公告计时器结束时
	 * @param MsgToRemove 需要移除的文本指针
	 */
	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	//玩家淘汰公告文本数组
	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package;}
};
