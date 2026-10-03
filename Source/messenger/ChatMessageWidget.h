#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMessageWidget.generated.h"

class UTextBlock;

UCLASS()
class MESSENGER_API UChatMessageWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	int32 MessageID = -1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EditedTextIndicator;

	void Setup(int32 InID, FString Sender, FString Text, FString Time, bool bIsMine, bool bIsEdited, bool bIsRead);

	UPROPERTY()
	class ULoginWidget* ParentWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* DownloadButton;

	UFUNCTION()
	void OnDownloadClicked();

	FString AttachedFileName;

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;

	UPROPERTY(meta = (BindWidget))
	class UButton* EditButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnEditButtonClicked();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReadStatusText;
};