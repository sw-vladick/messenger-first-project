#include "UserListItemWidget.h"
#include "Components/TextBlock.h"

void UUserListItemWidget::Setup(const FString& InUsername)
{
    if (UserNameText)
    {
        UserNameText->SetText(FText::FromString(InUsername));
    }
}