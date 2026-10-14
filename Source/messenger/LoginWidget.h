#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/editabletextBox.h"
#include "components/button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateBrush.h"
#include "ServerResponseRouter.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "UserListItemWidget.h"
#include "ChatMessageWidget.h"
#include "Components/MultiLineEditableText.h"
#include "Components/ScrollBoxSlot.h"
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
	void HandleLoginResponse(FString Status, bool bIsVerified);

	UFUNCTION()
	void HandleRegisterResponse(FString Status, bool bIsVerified);

	UFUNCTION()
	void HandleSendCodeResponse(FString Status);

	UFUNCTION()
	void HandleVerifyCodeResponse(FString Status);

	UFUNCTION()
	void OnSendCodeClicked();

	UFUNCTION()
	void OnVerifyCodeClicked();

	UFUNCTION()
	void HandleChatHistory(const TArray<FChatMessageData>& Messages);

	UFUNCTION()
	void HandleUpdateChat();

	virtual void NativeConstruct() override;

	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION()
	void OnRegistrationButtonClicked();

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

	UFUNCTION()
	void OnUserSelected(FString SelectedUsername);

	FString CurrentChatUser;

	UPROPERTY(meta = (BindWidgetOptional))
	class UScrollBox* MessagesScrollBox;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UChatMessageWidget> ChatMessageClass;

	UPROPERTY(meta = (BindWidgetOptional))
	class UMultiLineEditableText* MessageInputBox;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* SendMessageButton;

	UFUNCTION()
	void OnSendMessageClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* LogoutButton;

	UFUNCTION()
	void OnLogoutButtonClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* AttachFileButton;

	UFUNCTION()
	void OnAttachFileButtonClicked();

	FButtonStyle OriginalSendCodeStyle;
	bool bIsSendCodeStyleSaved = false;
public:
	void StartEditingMessage(int32 MsgID, FString OldText);

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

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* Input_Email;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* Input_Code;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_SendCode;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Verify;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_SendCodeBtn;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ErrorText_Email;

	FTimerHandle SendCodeTimerHandle;
	int32 SendCodeCountdown;

	UFUNCTION()
	void UpdateSendCodeTimer();
private:
		int32 CurrentEditingMessageID = -1;
};
