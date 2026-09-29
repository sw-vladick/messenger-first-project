#include "LoginWidget.h"

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
		NetSubsystem->ResponseRouter->OnServerResponse.AddDynamic(this, &ULoginWidget::OnServerResponseReceived);

		NetSubsystem->ResponseRouter->OnSearchResultsDelegate.AddDynamic(this, &ULoginWidget::UpdateSearchResults);
	}
	if (SearchInputText) {
		SearchInputText->OnTextChanged.AddDynamic(this, &ULoginWidget::OnSearchTextChanged);
	}
}

void ULoginWidget::OnLoginButtonClicked()
{
	if (!LoginInput || !PasswordInput)
	{
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
		return;
	}
	
	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();

	if (NetSubsystem) {
		if (NetSubsystem->ConnectToServer(TEXT("127.0.0.1"), NetConst::PORT)) {
			TSharedPtr<FJsonObject> InnerJson = MakeShareable(new FJsonObject());
			InnerJson->SetStringField(TEXT("Username"), Username);
			InnerJson->SetStringField(TEXT("Password"), Password);

			FString InnerPayload;
			TSharedRef<TJsonWriter<>> InnerWriter = TJsonWriterFactory<>::Create(&InnerPayload);
			FJsonSerializer::Serialize(InnerJson.ToSharedRef(), InnerWriter);

			FString EncryptedBase64 = NetSubsystem->CryptoClient->EncryptAES(InnerPayload);

			TSharedPtr<FJsonObject> EnvelopeJson = MakeShareable(new FJsonObject());
			EnvelopeJson->SetStringField(TEXT("Command"), TEXT("LOGIN"));
			EnvelopeJson->SetStringField(TEXT("payload"), EncryptedBase64);

			FString FinalString;
			TSharedRef<TJsonWriter<>> EnvWriter = TJsonWriterFactory<>::Create(&FinalString);
			FJsonSerializer::Serialize(EnvelopeJson.ToSharedRef(), EnvWriter);

			NetSubsystem->SendTextMessage(FinalString);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Не вдалося підключитися до сервера!"));
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
				TSharedPtr<FJsonObject> InnerJson = MakeShareable(new FJsonObject());
				InnerJson->SetStringField(TEXT("Username"), Username);
				InnerJson->SetStringField(TEXT("Password"), Password);

				FString InnerPayload;
				TSharedRef<TJsonWriter<>> InnerWriter = TJsonWriterFactory<>::Create(&InnerPayload);
				FJsonSerializer::Serialize(InnerJson.ToSharedRef(), InnerWriter);

				FString EncryptedBase64 = NetSubsystem->CryptoClient->EncryptAES(InnerPayload);

				TSharedPtr<FJsonObject> EnvelopeJson = MakeShareable(new FJsonObject());
				EnvelopeJson->SetStringField(TEXT("Command"), TEXT("REGISTER"));
				EnvelopeJson->SetStringField(TEXT("payload"), EncryptedBase64);

				FString FinalString;
				TSharedRef<TJsonWriter<>> EnvWriter = TJsonWriterFactory<>::Create(&FinalString);
				FJsonSerializer::Serialize(EnvelopeJson.ToSharedRef(), EnvWriter);

				NetSubsystem->SendTextMessage(FinalString);
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Не вдалося підключитися до сервера при реєстрації!"));
			}
		}
	}
	else {
		ErrorText_Password_Reg->SetText(FText::FromString(TEXT("Пароль має містити від 8-64 символів, велику літеру, цифру та спецсимвол.")));
		ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Visible);
	}
	
}

void ULoginWidget::OnServerResponseReceived(FString Command, FString StatusData)
{
	UE_LOG(LogTemp, Warning, TEXT("[WIDGET] Отримано сигнал від роутера!"));

	FString RealStatus = StatusData;

	if (Command == TEXT("LOGIN_RESPONSE") || Command == TEXT("REGISTER_RESPONSE") || Command == TEXT("SEARCH_RESPONSE"))
	{
		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
		if (NetSubsystem && NetSubsystem->CryptoClient)
		{
			FString DecryptedStr = NetSubsystem->CryptoClient->DecryptAES(StatusData);

			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DecryptedStr);
			TSharedPtr<FJsonObject> DecryptedJson;
			if (FJsonSerializer::Deserialize(Reader, DecryptedJson) && DecryptedJson.IsValid())
			{
				RealStatus = DecryptedJson->GetStringField(TEXT("status"));

				if (Command == TEXT("SEARCH_RESPONSE"))
				{
					const TArray<TSharedPtr<FJsonValue>>* UsersArray;
					if (DecryptedJson->TryGetArrayField(TEXT("users"), UsersArray))
					{
						TArray<FString> FoundUsers;
						for (const TSharedPtr<FJsonValue>& Val : *UsersArray)
						{
							FoundUsers.Add(Val->AsString());
						}
						UpdateSearchResults(FoundUsers);
					}
				}
			}
		}
	}

	if (Command == TEXT("LOGIN_RESPONSE"))
	{
		if (RealStatus == TEXT("success")) {
			ErrorText_Password->SetVisibility(ESlateVisibility::Hidden);
			WidgetSwitcher_0->SetActiveWidgetIndex(2);
		}
		else if (RealStatus == TEXT("locked")) {
			ErrorText_Password->SetText(FText::FromString(TEXT("Забагато спроб, спробуйте пізніше!")));
			ErrorText_Password->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			ErrorText_Password->SetText(FText::FromString(TEXT("Невірний логін або пароль!")));
			ErrorText_Password->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (Command == TEXT("REGISTER_RESPONSE"))
	{
		if (RealStatus == TEXT("success")) {
			ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Hidden);
			WidgetSwitcher_0->SetActiveWidgetIndex(2);
		}
		else if (RealStatus == TEXT("locked")) {
			ErrorText_Password_Reg->SetText(FText::FromString(TEXT("Забагато спроб, спробуйте пізніше!")));
			ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			ErrorText_Password_Reg->SetText(FText::FromString(TEXT("Користувач вже існує або помилка!")));
			ErrorText_Password_Reg->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ULoginWidget::OnSearchCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter) {
		FString SearchQuery = SearchInputText->GetText().ToString();
		if (SearchQuery.IsEmpty()) return;

		UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
		if (NetSubsystem && NetSubsystem->CryptoClient) {

			TSharedPtr<FJsonObject> InnerJson = MakeShareable(new FJsonObject());
			InnerJson->SetStringField(TEXT("username"), SearchQuery);

			FString InnerPayload;
			TSharedRef<TJsonWriter<>> InnerWriter = TJsonWriterFactory<>::Create(&InnerPayload);
			FJsonSerializer::Serialize(InnerJson.ToSharedRef(), InnerWriter);

			FString EncryptedBase64 = NetSubsystem->CryptoClient->EncryptAES(InnerPayload);

			TSharedPtr<FJsonObject> EnvelopeJson = MakeShareable(new FJsonObject());
			EnvelopeJson->SetStringField(TEXT("Command"), TEXT("SEARCH_USER"));
			EnvelopeJson->SetStringField(TEXT("payload"), EncryptedBase64);

			FString FinalString;
			TSharedRef<TJsonWriter<>> EnvWriter = TJsonWriterFactory<>::Create(&FinalString);
			FJsonSerializer::Serialize(EnvelopeJson.ToSharedRef(), EnvWriter);

			NetSubsystem->SendTextMessage(FinalString);
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

			ChatsScrollBox->AddChild(NewItem);
		}
	}
}

void ULoginWidget::OnSearchTextChanged(const FText& Text)
{
	FString SearchQuery = Text.ToString();

	if (SearchQuery.IsEmpty()) {
		UpdateSearchResults(TArray<FString>());
		return;
	}

	UNetworkManagerSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetworkManagerSubsystem>();
	if (NetSubsystem && NetSubsystem->CryptoClient) {
		TSharedPtr<FJsonObject> InnerJson = MakeShareable(new FJsonObject());
		InnerJson->SetStringField(TEXT("username"), SearchQuery);

		FString InnerPayload;
		TSharedRef<TJsonWriter<>> InnerWriter = TJsonWriterFactory<>::Create(&InnerPayload);
		FJsonSerializer::Serialize(InnerJson.ToSharedRef(), InnerWriter);

		FString EncryptedBase64 = NetSubsystem->CryptoClient->EncryptAES(InnerPayload);

		TSharedPtr<FJsonObject> EnvelopeJson = MakeShareable(new FJsonObject());
		EnvelopeJson->SetStringField(TEXT("Command"), TEXT("SEARCH_USER"));
		EnvelopeJson->SetStringField(TEXT("payload"), EncryptedBase64);

		FString FinalString;
		TSharedRef<TJsonWriter<>> EnvWriter = TJsonWriterFactory<>::Create(&FinalString);
		FJsonSerializer::Serialize(EnvelopeJson.ToSharedRef(), EnvWriter);

		NetSubsystem->SendTextMessage(FinalString);
	}
}