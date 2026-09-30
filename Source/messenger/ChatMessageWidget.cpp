#include "ChatMessageWidget.h"
#include "Components/TextBlock.h"

void UChatMessageWidget::Setup(const FString& InSender, const FString& InText, const FString& InTime, bool bIsMine)
{
	if (MessageText) {
		MessageText->SetText(FText::FromString(InText));
	}
	if (TimeText) {
		TimeText->SetText(FText::FromString(InTime));
	}
}