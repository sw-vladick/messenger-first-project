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
	void Setup(const FString& InSender, const FString& InText, const FString& InTime, bool bIsMine);

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
};