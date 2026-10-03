#include "ChatMessageWidget.h"
#include "LoginWidget.h"
#include "Components/TextBlock.h"
#include "NetworkManagerSubsystem.h"
#include "Dom/JsonObject.h"

void UChatMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditButton) {
		EditButton->OnClicked.AddDynamic(this, &UChatMessageWidget::OnEditButtonClicked);
	}
	if (DownloadButton) {
		DownloadButton->OnClicked.AddDynamic(this, &UChatMessageWidget::OnDownloadClicked);
	}
}

void UChatMessageWidget::Setup(int32 InID, FString Sender, FString Text, FString Time, bool bIsMine, bool bIsEdited, bool bIsRead)
{
	MessageID = InID;
	if (EditedTextIndicator) {
		EditedTextIndicator->SetVisibility(bIsEdited ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (MessageText) {
		if (Text.StartsWith(TEXT("[FILE] "))) {
			AttachedFileName = Text.RightChop(7);

			MessageText->SetText(FText::FromString(TEXT("📎 Файл: ") + AttachedFileName));

			MessageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f, 1.0f)));

			if (EditButton) EditButton->SetVisibility(ESlateVisibility::Collapsed);

			if (DownloadButton) DownloadButton->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			MessageText->SetText(FText::FromString(Text));

			MessageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));

			if (DownloadButton) DownloadButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	if (TimeText) {
		TimeText->SetText(FText::FromString(Time));
	}

	if (EditButton) {
		if (bIsMine) {
			EditButton->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			EditButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	if (ReadStatusText) {
		if (bIsMine) {
			ReadStatusText->SetVisibility(ESlateVisibility::Visible);

			ReadStatusText->SetText(FText::FromString(bIsRead ? TEXT("✓✓") : TEXT("✓")));
		}
		else {
			ReadStatusText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UChatMessageWidget::OnEditButtonClicked()
{

	if(ParentWidget && MessageText) {
		ParentWidget->StartEditingMessage(MessageID, MessageText->GetText().ToString());
	}
}

void UChatMessageWidget::OnDownloadClicked()
{
	if (AttachedFileName.IsEmpty()) return;

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		if (DownloadButton) DownloadButton->SetIsEnabled(false);

		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		Json->SetStringField(TEXT("file_name"), AttachedFileName);

		NetSubsystem->SendEncryptedCommand(TEXT("DOWNLOAD_FILE"), Json);

	}
}