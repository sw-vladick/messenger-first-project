#include "UserListItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UUserListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MainButton)
	{
		MainButton->OnClicked.AddDynamic(this, &UUserListItemWidget::OnItemButtonClicked);
	}
}

void UUserListItemWidget::Setup(const FString& InUsername)
{
	CurrentUsername = InUsername;

	if (UserNameText)
	{
		UserNameText->SetText(FText::FromString(InUsername));
	}
}

void UUserListItemWidget::OnItemButtonClicked()
{
	MainButton->SetIsEnabled(false);

	OnUserClicked.Broadcast(CurrentUsername);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (MainButton) {
				MainButton->SetIsEnabled(true);
			}
		}, 0.5f, false);
}