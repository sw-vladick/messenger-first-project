#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/editabletextBox.h"
#include "components/button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateBrush.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "NetworkManagerSubsystem.h"
#include "NetworkConstants.h"
#include "Serialization/JsonSerializer.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "UserListItemWidget.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class MESSENGER_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
	void UpdatePasswordButtonIcon(bool bIsVisible);

protected:
	UFUNCTION()
	void OnServerResponseReceived(FString Command, FString Status);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network | Auth")
	bool IsPasswordValid(const FString& Password);

	UPROPERTY(meta = (BindWidget))
	class UButton* TogglePasswordButton;

	UPROPERTY(EditAnywhere, Category = "UI | Icons")
	class UTexture2D* OpenEyePass;

	UPROPERTY(EditAnywhere, Category = "UI | Icons")
	class UTexture2D* CloseEyePass;

	UFUNCTION()
	void OnTogglePasswordClicked();

	UFUNCTION()
	void OnGoToRegisterClicked();

	UFUNCTION()
	void OnGoToLoginClicked();

	UFUNCTION(BlueprintCallable, Category = "Network | Auth")
	void SendRegisterRequest(const FString& Username, const FString& Password);

	UFUNCTION()
	void OnSearchCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserListItemWidget> UserListItemClass;

	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);
public:
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* LoginInput;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordInput;

	UPROPERTY(meta = (BindWidget))
	UButton* LoginButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ErrorText_Login;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ErrorText_Password;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher_0;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonToLogin;

	UPROPERTY(meta = (BindWidget))
	class UButton* RegistrationButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonToRegistration;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* LoginInputReg;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* PasswordInputReg;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* ErrorText_Login_Reg;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* ErrorText_Password_Reg;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SearchInputText;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollChat;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ChatsScrollBox;

	UFUNCTION()
	void UpdateSearchResults(const TArray<FString>& FoundUsers);
};
