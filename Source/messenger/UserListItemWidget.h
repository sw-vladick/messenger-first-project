#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserListItemWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserItemClicked, FString, Username);

UCLASS()
class MESSENGER_API UUserListItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    void Setup(const FString& InUsername);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUserItemClicked OnUserClicked;
protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* UserNameText;

    UPROPERTY(meta = (BindWidget))
    UButton* MainButton;

    UFUNCTION()
    void OnItemButtonClicked();

private:
    FString CurrentUsername;
};
