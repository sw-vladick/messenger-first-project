#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserListItemWidget.generated.h"

class UTextBlock;

UCLASS()
class MESSENGER_API UUserListItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    void Setup(const FString& InUsername);
protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* UserNameText;
};
