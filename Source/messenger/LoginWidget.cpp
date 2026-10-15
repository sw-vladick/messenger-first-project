#include "LoginWidget.h"
#include "NetworkManagerSubsystem.h"
#include "NetworkConstants.h"
#include "Dom/JsonObject.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

void ULoginWidget::UpdatePasswordButtonIcon(bool bIsVisible)
{
	if (!TogglePasswordButton) return;

	UTexture2D* TexToUse = bIsVisible ? OpenEyePass : CloseEyePass;

	if (TexToUse) {
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(TexToUse);

		FButtonStyle NewStyle = TogglePasswordButton->GetStyle();
		NewStyle.SetNormal(NewBrush);
		NewStyle.SetPressed(NewBrush);
		NewStyle.SetHovered(NewBrush);

		TogglePasswordButton->SetStyle(NewStyle);
	}
}

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginButton) {
		LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLoginButtonClicked);
	}
	if (TogglePasswordButton)
	{
		TogglePasswordButton->OnClicked.AddDynamic(this, &ULoginWidget::OnTogglePasswordClicked);

		UpdatePasswordButtonIcon(false);
	}
	if (ButtonToRegistration) {
		ButtonToRegistration->OnClicked.AddDynamic(this, &ULoginWidget::OnGoToRegisterClicked);
	}
	if (ButtonToLogin) {
		ButtonToLogin->OnClicked.AddDynamic(this, &ULoginWidget::OnGoToLoginClicked);
	}
	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem && NetSubsystem->ResponseRouter)
	{
		NetSubsystem->ResponseRouter->OnLoginResponseDelegate.AddDynamic(this, &ULoginWidget::HandleLoginResponse);
		NetSubsystem->ResponseRouter->OnRegisterResponseDelegate.AddDynamic(this, &ULoginWidget::HandleRegisterResponse);
		NetSubsystem->ResponseRouter->OnSearchResultsDelegate.AddDynamic(this, &ULoginWidget::UpdateSearchResults);
		NetSubsystem->ResponseRouter->OnChatHistoryDelegate.AddDynamic(this, &ULoginWidget::HandleChatHistory);
		NetSubsystem->ResponseRouter->OnUpdateChatDelegate.AddDynamic(this, &ULoginWidget::HandleUpdateChat);
		NetSubsystem->ResponseRouter->OnSendCodeResponseDelegate.AddDynamic(this, &ULoginWidget::HandleSendCodeResponse);
		NetSubsystem->ResponseRouter->OnVerifyCodeResponseDelegate.AddDynamic(this, &ULoginWidget::HandleVerifyCodeResponse);
		NetSubsystem->ResponseRouter->OnGoogleLoginResponseDelegate.AddDynamic(this, &ULoginWidget::HandleGoogleLoginResponse);
	}
	if (Btn_SendCode) {
		Btn_SendCode->OnClicked.AddDynamic(this, &ULoginWidget::OnSendCodeClicked);
	}
	if (Btn_Verify) {
		Btn_Verify->OnClicked.AddDynamic(this, &ULoginWidget::OnVerifyCodeClicked);
	}
	if (SearchInputText) {
		SearchInputText->OnTextChanged.AddDynamic(this, &ULoginWidget::OnSearchTextChanged);
	}
	if (SendMessageButton) {
		SendMessageButton->OnClicked.AddDynamic(this, &ULoginWidget::OnSendMessageClicked);
	}
	if (RegistrationButton) {
		RegistrationButton->OnClicked.AddDynamic(this, &ULoginWidget::OnRegistrationButtonClicked);
	}
	if (LogoutButton) {
		LogoutButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLogoutButtonClicked);
	}
	if (AttachFileButton) {
		AttachFileButton->OnClicked.AddDynamic(this, &ULoginWidget::OnAttachFileButtonClicked);
	}
	if (Btn_GoogleLogin) {
		Btn_GoogleLogin->OnClicked.AddDynamic(this, &ULoginWidget::OnGoogleLoginClicked);
	}
	if (GoogleWebBrowser) {
		GoogleWebBrowser->OnUrlChanged.AddDynamic(this, &ULoginWidget::OnUrlChanged);
	}
}
FReply ULoginWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if (MessageInputBox && MessageInputBox->HasKeyboardFocus())
		{
			if (!InKeyEvent.IsShiftDown())
			{
				OnSendMessageClicked();
				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void ULoginWidget::OnLoginButtonClicked()
{
	if (LoginButton) {
		LoginButton->SetIsEnabled(false);
	}
	if (!LoginInput || !PasswordInput)
	{
		if (LoginButton) LoginButton->SetIsEnabled(true);
		return;
	}
	FString Username = LoginInput->GetText().ToString();
	FString Password = PasswordInput->GetText().ToString();
	bool bHasError = false;

	if (Username.IsEmpty()) {
		ErrorText_Login->SetText(FText::FromString(TEXT("Введіть логін")));
		ErrorText_Login->SetVisibility(ESlateVisibility::Visible);
		bHasError = true;
	}
	else {
		ErrorText_Login->SetVisibility(ESlateVisibility::Hidden);
	}

	if (Password.IsEmpty()) {
		ErrorText_Password->SetText(FText::FromString(TEXT("Введіть пароль")));
		ErrorText_Password->SetVisibility(ESlateVisibility::Visible);
		bHasError = true;
	}
	else {
		ErrorText_Password->SetVisibility(ESlateVisibility::Hidden);
	}

	if (bHasError) {
		if (LoginButton) LoginButton->SetIsEnabled(true);
		return;
	}

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();

	if (NetSubsystem) {
		if (NetSubsystem->ConnectToServer(TEXT("127.0.0.1"), NetConst::PORT)) {
			TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
			Json->SetStringField(TEXT("Username"), Username);
			Json->SetStringField(TEXT("Password"), Password);

			NetSubsystem->SendEncryptedCommand(TEXT("LOGIN"), Json);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Не вдалося підключитися до сервера!"));
			if (LoginButton) LoginButton->SetIsEnabled(true);
		}
	}
}

bool ULoginWidget::IsPasswordValid(const FString& Password)
{
	if (Password.Len() < 8 || Password.Len()>64) {
		return false;
	}
	bool bHasUpper = false, bHasLower = false, bHasDigit = false, bHasSpecial = false;
	for (TCHAR Char : Password) {
		if (Char >= 'A' && Char <= 'Z') {
			bHasUpper = true;
		}
		else if (Char >= 'a' && Char <= 'z') bHasLower = true;
		else if (Char >= '0' && Char <= '9') bHasDigit = true;
		else bHasSpecial = true;
	}
	return bHasUpper && bHasLower && bHasDigit && bHasSpecial;
}

void ULoginWidget::OnTogglePasswordClicked()
{
	if (!PasswordInput) return;

	bool bIsCurrentlyPassword = PasswordInput->GetIsPassword();

	bool bNewVisibility = !bIsCurrentlyPassword;
	PasswordInput->SetIsPassword(bNewVisibility);

	UpdatePasswordButtonIcon(!bNewVisibility);
}

void ULoginWidget::OnGoToRegisterClicked()
{
	if (!ButtonToRegistration) return;
	WidgetSwitcher_0->SetActiveWidgetIndex(1);
}

void ULoginWidget::OnGoToLoginClicked()
{
	if (!ButtonToLogin) return;
	WidgetSwitcher_0->SetActiveWidgetIndex(0);
}

void ULoginWidget::SendRegisterRequest(const FString& Username, const FString& Password)
{
	if (IsPasswordValid(Password)) {
		ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Hidden);
		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();

		if (NetSubsystem) {
			if (NetSubsystem->ConnectToServer(TEXT("127.0.0.1"), NetConst::PORT)) {
				TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
				Json->SetStringField(TEXT("Username"), Username);
				Json->SetStringField(TEXT("Password"), Password);

				NetSubsystem->SendEncryptedCommand(TEXT("REGISTER"), Json);
			}
			else {
				if (RegistrationButton) RegistrationButton->SetIsEnabled(true);
			}
		}
	}
	else {
		ErrorText_Password_Reg->SetText(FText::FromString(TEXT("Пароль має містити від 8-64 символів, велику літеру, цифру та спецсимвол.")));
		ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Visible);

		if (RegistrationButton) RegistrationButton->SetIsEnabled(true);
	}
}

void ULoginWidget::HandleLoginResponse(FString Status, bool bIsVerified)
{
	if (LoginButton) {
		LoginButton->SetIsEnabled(true);
	}

	if (Status == TEXT("success")) {
		ErrorText_Password->SetVisibility(ESlateVisibility::Hidden);

		if (bIsVerified) {
			WidgetSwitcher_0->SetActiveWidgetIndex(2);

			UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
			if (NetSubsystem) {
				TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
				Json->SetStringField(TEXT("request"), TEXT("get_chats"));
				NetSubsystem->SendEncryptedCommand(TEXT("GET_RECENT_CHATS"), Json);
			}
		}
		else {
			WidgetSwitcher_0->SetActiveWidgetIndex(3);
		}
	}
	else if (Status == TEXT("locked")) {
		ErrorText_Password->SetText(FText::FromString(TEXT("Забагато спроб, спробуйте пізніше!")));
		ErrorText_Password->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		ErrorText_Password->SetText(FText::FromString(TEXT("Невірний логін або пароль!")));
		ErrorText_Password->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULoginWidget::HandleRegisterResponse(FString Status, bool bIsVerified)
{
	if (RegistrationButton) {
		RegistrationButton->SetIsEnabled(true);
	}

	if (Status == TEXT("success")) {
		ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Hidden);
		WidgetSwitcher_0->SetActiveWidgetIndex(3);
	}
	else if (Status == TEXT("locked")) {
		ErrorText_Password_Reg->SetText(FText::FromString(TEXT("Забагато спроб, спробуйте пізніше!")));
		ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULoginWidget::HandleUpdateChat()
{
	if (!CurrentChatUser.IsEmpty()) {
		OnUserSelected(CurrentChatUser);
	}
}

void ULoginWidget::HandleChatHistory(const TArray<FChatMessageData>& Messages)
{
	if (!MessagesScrollBox || !ChatMessageClass) return;

	MessagesScrollBox->ClearChildren();

	for (const FChatMessageData& MsgData : Messages)
	{
		bool bIsMine = (MsgData.Sender != CurrentChatUser);

		UChatMessageWidget* NewMsg = CreateWidget<UChatMessageWidget>(this, ChatMessageClass);
		if (NewMsg)
		{
			NewMsg->ParentWidget = this;
			NewMsg->Setup(MsgData.ID, MsgData.Sender, MsgData.Text, MsgData.Timestamp, bIsMine, MsgData.bIsEdited, MsgData.bIsRead);

			UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(MessagesScrollBox->AddChild(NewMsg));
			if (ScrollSlot)
			{
				ScrollSlot->SetHorizontalAlignment(bIsMine ? HAlign_Right : HAlign_Left);
				ScrollSlot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 5.0f));
			}
		}
	}
	MessagesScrollBox->ScrollToEnd();
}

void ULoginWidget::OnSearchCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter) {
		FString SearchQuery = SearchInputText->GetText().ToString();
		if (SearchQuery.IsEmpty()) return;

		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
		if (NetSubsystem) {
			TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
			Json->SetStringField(TEXT("username"), SearchQuery);

			NetSubsystem->SendEncryptedCommand(TEXT("SEARCH_USER"), Json);
		}
	}
}

void ULoginWidget::UpdateSearchResults(const TArray<FString>& FoundUsers)
{
	if (!ChatsScrollBox || !UserListItemClass) return;

	UE_LOG(LogTemp, Warning, TEXT("[UI] Будемо малювати кнопок: %d"), FoundUsers.Num());

	ChatsScrollBox->ClearChildren();

	for (const FString& Username : FoundUsers)
	{
		UUserListItemWidget* NewItem = CreateWidget<UUserListItemWidget>(this, UserListItemClass);

		if (NewItem)
		{
			NewItem->Setup(Username);

			NewItem->OnUserClicked.AddDynamic(this, &ULoginWidget::OnUserSelected);

			ChatsScrollBox->AddChild(NewItem);
		}
	}
}

void ULoginWidget::OnSearchTextChanged(const FText& Text)
{
	FString SearchQuery = Text.ToString();
	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();

	if (NetSubsystem) {
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());

		if (SearchQuery.IsEmpty()) {
			Json->SetStringField(TEXT("request"), TEXT("get_chats"));
			NetSubsystem->SendEncryptedCommand(TEXT("GET_RECENT_CHATS"), Json);
		}
		else {
			Json->SetStringField(TEXT("username"), SearchQuery);
			NetSubsystem->SendEncryptedCommand(TEXT("SEARCH_USER"), Json);
		}
	}
}

void ULoginWidget::OnUserSelected(FString SelectedUsername)
{
	CurrentChatUser = SelectedUsername;

	if (SearchInputText) SearchInputText->SetText(FText::GetEmpty());
	if (MessagesScrollBox) MessagesScrollBox->ClearChildren();

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		Json->SetStringField(TEXT("target_user"), SelectedUsername);
		NetSubsystem->SendEncryptedCommand(TEXT("GET_CHAT_HISTORY"), Json);
	}
}

void ULoginWidget::OnSendMessageClicked()
{
	if (!MessageInputBox || CurrentChatUser.IsEmpty()) return;

	FString MessageText = MessageInputBox->GetText().ToString();

	if (MessageText.IsEmpty()) return;

	MessageText = MessageText.TrimStartAndEnd();

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		FString CommandName = (CurrentEditingMessageID == -1) ? TEXT("SEND_MESSAGE") : TEXT("EDIT_MESSAGE");

		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		Json->SetStringField(TEXT("target_user"), CurrentChatUser);
		Json->SetStringField(TEXT("text"), MessageText);
		if (CurrentEditingMessageID != -1) {
			Json->SetNumberField(TEXT("message_id"), CurrentEditingMessageID);
		}

		NetSubsystem->SendEncryptedCommand(CommandName, Json);

		MessageInputBox->SetText(FText::GetEmpty());
		CurrentEditingMessageID = -1;
	}
}

void ULoginWidget::OnRegistrationButtonClicked()
{
	if (RegistrationButton) {
		RegistrationButton->SetIsEnabled(false);
	}

	if (!LoginInputReg || !PasswordInputReg) {
		if (RegistrationButton) RegistrationButton->SetIsEnabled(true);
		return;
	}

	FString Username = LoginInputReg->GetText().ToString();
	FString Password = PasswordInputReg->GetText().ToString();

	SendRegisterRequest(Username, Password);
}

void ULoginWidget::StartEditingMessage(int32 MsgID, FString OldText)
{
	CurrentEditingMessageID = MsgID;

	if (MessageInputBox) {
		MessageInputBox->SetText(FText::FromString(OldText));

		MessageInputBox->SetKeyboardFocus();
	}
}

void ULoginWidget::OnLogoutButtonClicked()
{

	CurrentChatUser = TEXT("");
	CurrentEditingMessageID = -1;

	if (MessagesScrollBox) MessagesScrollBox->ClearChildren();
	if (ChatsScrollBox) ChatsScrollBox->ClearChildren();

	if (SearchInputText) SearchInputText->SetText(FText::GetEmpty());
	if (MessageInputBox) MessageInputBox->SetText(FText::GetEmpty());

	if (LoginInput) LoginInput->SetText(FText::GetEmpty());
	if (PasswordInput) PasswordInput->SetText(FText::GetEmpty());

	if (WidgetSwitcher_0) {
		WidgetSwitcher_0->SetActiveWidgetIndex(0);
	}

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		NetSubsystem->SendEncryptedCommand(TEXT("LOGOUT"));
	}
}

void ULoginWidget::OnAttachFileButtonClicked()
{
	if (CurrentChatUser.IsEmpty()) return;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform) {
		TArray<FString> OutFileNames;

		bool bOpened = DesktopPlatform->OpenFileDialog(
			nullptr,
			TEXT("Оберіть файл для відправки"),
			TEXT(""),
			TEXT(""),
			TEXT("Всі файли (*.*)|*.*|Зображення (*.png;*.jpg)|*.png;*.jpg"),
			EFileDialogFlags::None,
			OutFileNames
		);

		if (bOpened && OutFileNames.Num() > 0) {
			FString SelectedFilePath = OutFileNames[0];

			UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
			if (NetSubsystem) {
				NetSubsystem->SendFileToServer(SelectedFilePath, CurrentChatUser);
			}
		}
	}
}

void ULoginWidget::OnSendCodeClicked()
{
	if (!Input_Email) return;

	FString Email = Input_Email->GetText().ToString().TrimStartAndEnd();
	if (Email.IsEmpty()) return;

	if (Btn_SendCode) {
		if (!bIsSendCodeStyleSaved) {
			OriginalSendCodeStyle = Btn_SendCode->GetStyle();
			bIsSendCodeStyleSaved = true;
		}

		Btn_SendCode->SetIsEnabled(false);

		FButtonStyle DisabledStyle = OriginalSendCodeStyle;
		FSlateBrush EmptyBrush;
		EmptyBrush.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
		DisabledStyle.SetDisabled(EmptyBrush);
		Btn_SendCode->SetStyle(DisabledStyle);
	}

	if (Text_SendCodeBtn) {
		Text_SendCodeBtn->SetVisibility(ESlateVisibility::Visible);
	}

	SendCodeCountdown = 60;
	GetWorld()->GetTimerManager().SetTimer(SendCodeTimerHandle, this, &ULoginWidget::UpdateSendCodeTimer, 1.0f, true);
	UpdateSendCodeTimer();

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		Json->SetStringField(TEXT("email"), Email);

		NetSubsystem->SendEncryptedCommand(TEXT("SEND_VERIFICATION_CODE"), Json);
	}
}

void ULoginWidget::OnVerifyCodeClicked()
{
	if (!Input_Email || !Input_Code) return;

	FString Email = Input_Email->GetText().ToString().TrimStartAndEnd();
	FString Code = Input_Code->GetText().ToString().TrimStartAndEnd();

	if (Email.IsEmpty() || Code.IsEmpty()) return;

	if (Btn_Verify) {
		Btn_Verify->SetIsEnabled(false);
	}

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem) {
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		Json->SetStringField(TEXT("email"), Email);
		Json->SetStringField(TEXT("code"), Code);

		NetSubsystem->SendEncryptedCommand(TEXT("VERIFY_CODE"), Json);
	}
}

void ULoginWidget::HandleSendCodeResponse(FString Status)
{
	if (Status == TEXT("success")) {
		if (ErrorText_Email) ErrorText_Email->SetVisibility(ESlateVisibility::Hidden);
	}
	else if (Status == TEXT("locked")) {
		if (ErrorText_Email) {
			ErrorText_Email->SetText(FText::FromString(TEXT("Забагато помилок! Пошту заблоковано на 5 хв.")));
			ErrorText_Email->SetVisibility(ESlateVisibility::Visible);
		}

		if (Btn_SendCode) Btn_SendCode->SetIsEnabled(false);
		SendCodeCountdown = 300;
		GetWorld()->GetTimerManager().SetTimer(SendCodeTimerHandle, this, &ULoginWidget::UpdateSendCodeTimer, 1.0f, true);
	}
}

void ULoginWidget::HandleVerifyCodeResponse(FString Status)
{
	if (Btn_Verify) Btn_Verify->SetIsEnabled(true);

	if (Status == TEXT("success")) {
		if (ErrorText_Email) ErrorText_Email->SetVisibility(ESlateVisibility::Hidden);
		WidgetSwitcher_0->SetActiveWidgetIndex(2);

		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
		if (NetSubsystem) {
			TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
			Json->SetStringField(TEXT("request"), TEXT("get_chats"));
			NetSubsystem->SendEncryptedCommand(TEXT("GET_RECENT_CHATS"), Json);
		}
	}
	else if (Status == TEXT("locked")) {
		if (ErrorText_Email) {
			ErrorText_Email->SetText(FText::FromString(TEXT("Забагато помилок! Пошту заблоковано на 5 хв.")));
			ErrorText_Email->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else {
		if (ErrorText_Email) {
			ErrorText_Email->SetText(FText::FromString(TEXT("Невірний код!")));
			ErrorText_Email->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ULoginWidget::UpdateSendCodeTimer()
{
	SendCodeCountdown--;
	if (SendCodeCountdown > 0) {
		if (Text_SendCodeBtn) {
			Text_SendCodeBtn->SetText(FText::FromString(FString::Printf(TEXT("%d с"), SendCodeCountdown)));
		}
	}
	else {
		GetWorld()->GetTimerManager().ClearTimer(SendCodeTimerHandle);
		if (Btn_SendCode) {
            Btn_SendCode->SetIsEnabled(true);
            Btn_SendCode->SetStyle(OriginalSendCodeStyle); 
        }
        if (Text_SendCodeBtn) {
            Text_SendCodeBtn->SetText(FText::GetEmpty());
            Text_SendCodeBtn->SetVisibility(ESlateVisibility::Hidden);
        }
	}
}

void ULoginWidget::OnGoogleLoginClicked()
{
	if (WidgetSwitcher_0) {
		WidgetSwitcher_0->SetActiveWidgetIndex(4);
	}

	if (GoogleWebBrowser) {
		FString ClientID = TEXT("1087750433701-t197ngi71m3o8uu5p1ueeh1di8crh7qh.apps.googleusercontent.com");
		FString RedirectURI = TEXT("http://127.0.0.1");

		FString AuthUrl = FString::Printf(TEXT("https://accounts.google.com/o/oauth2/v2/auth?client_id=%s&redirect_uri=%s&response_type=code&scope=email%%20profile&prompt=select_account"), *ClientID, *RedirectURI);

		GoogleWebBrowser->LoadURL(AuthUrl);
	}
}

void ULoginWidget::OnUrlChanged(const FText& Text)
{
	FString CurrentUrl = Text.ToString();

	if (CurrentUrl.Contains(TEXT("127.0.0.1")))
	{
		FString SearchPattern = TEXT("code=");
		int32 CodeIndex = CurrentUrl.Find(SearchPattern);

		if (CodeIndex != INDEX_NONE)
		{
			FString CodeStr = CurrentUrl.RightChop(CodeIndex + SearchPattern.Len());

			int32 AmpersandIndex;
			if (CodeStr.FindChar('&', AmpersandIndex)) {
				CodeStr = CodeStr.Left(AmpersandIndex);
			}

			if (WidgetSwitcher_0) {
				WidgetSwitcher_0->SetActiveWidgetIndex(0);
			}

			UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
			if (NetSubsystem) {
				if (NetSubsystem->ConnectToServer(TEXT("127.0.0.1"), NetConst::PORT)) {

					TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
					Json->SetStringField(TEXT("code"), CodeStr);

					NetSubsystem->SendEncryptedCommand(TEXT("GOOGLE_LOGIN"), Json);
				}
				else {
					if (ErrorText_Login) {
						ErrorText_Login->SetText(FText::FromString(TEXT("Не вдалося підключитися до сервера!")));
						ErrorText_Login->SetVisibility(ESlateVisibility::Visible);
					}
				}
			}
		}
	}
}

void ULoginWidget::HandleGoogleLoginResponse(FString Status, bool bIsVerified)
{
	if (Status == TEXT("success")) {
		if (WidgetSwitcher_0) {
			WidgetSwitcher_0->SetActiveWidgetIndex(2);
		}

		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
		if (NetSubsystem) {
			TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
			Json->SetStringField(TEXT("request"), TEXT("get_chats"));
			NetSubsystem->SendEncryptedCommand(TEXT("GET_RECENT_CHATS"), Json);
		}
	}
	else {
		if (ErrorText_Login) {
			ErrorText_Login->SetText(FText::FromString(TEXT("Помилка авторизації через Google.")));
			ErrorText_Login->SetVisibility(ESlateVisibility::Visible);
		}
	}
}