#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

class UHorizontalBox;
class UTextBlock;

/**
 * 玩家淘汰公告HUD控件
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:

	/**
	 * 设置玩家淘汰公告
	 * @param AttackerName 淘汰者名字
	 * @param VictimName 被淘汰者名字
	 */
	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	//水平盒绑定
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AnnouncementBox;

	//公告文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;
};
