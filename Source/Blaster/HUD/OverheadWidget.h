#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

/**
 * 玩家头顶HUD控件
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	//头顶文字文本
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

	/**
	 * 设置头顶文字文本
	 * @param TextToDisplay 文字文本字符串
	 */
	void SetDisplayText(FString TextToDisplay);
	
protected:
	
	//组件销毁
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
