#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

/**
 * 公告HUD控件
 */
UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:

	//热身倒计时绑定
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTime;

	//公告文本绑定
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	//信息文本绑定
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;
};
