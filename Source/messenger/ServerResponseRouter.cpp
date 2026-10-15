#include "ServerResponseRouter.h"
#include "NetworkManagerSubsystem.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UServerResponseRouter::UServerResponseRouter() {}

void UServerResponseRouter::ProcessServerResponse(FString Command, FString StatusData)
{
	UNetworkManagerSubsystem* NetSubsystem = Cast<UNetworkManagerSubsystem>(GetOuter());
	if (!NetSubsystem || !NetSubsystem->CryptoClient) return;

	FString DecryptedStr = NetSubsystem->CryptoClient->DecryptAES(StatusData);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DecryptedStr);
	TSharedPtr<FJsonObject> Json;

	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid()) return;


	if (Command == TEXT("HANDSHAKE_RESPONSE")) {
		OnHandshakeResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")));
	}
	if (Command == TEXT("LOGIN_RESPONSE")) {
		bool bIsVerified = false;
		Json->TryGetBoolField(TEXT("is_verified"), bIsVerified);
		OnLoginResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")), bIsVerified);
	}
	else if (Command == TEXT("REGISTER_RESPONSE")) {
		bool bIsVerified = false;
		Json->TryGetBoolField(TEXT("is_verified"), bIsVerified);
		OnRegisterResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")), bIsVerified);
	}
	else if (Command == TEXT("SEND_VERIFICATION_CODE_RESPONSE")) {
		OnSendCodeResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")));
	}
	else if (Command == TEXT("VERIFY_CODE_RESPONSE")) {
		OnVerifyCodeResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")));
	}
	else if (Command == TEXT("GOOGLE_LOGIN_RESPONSE")) {
		OnGoogleLoginResponseDelegate.Broadcast(Json->GetStringField(TEXT("status")), true);
	}
	else if (Command == TEXT("SEARCH_RESPONSE") || Command == TEXT("RECENT_CHATS_RESPONSE")) {
		TArray<FString> FoundUsers;
		const TArray<TSharedPtr<FJsonValue>>* UsersArray;
		if (Json->TryGetArrayField(TEXT("users"), UsersArray)) {
			for (const auto& Val : *UsersArray) {
				FoundUsers.Add(Val->AsString());
			}
		}
		OnSearchResultsDelegate.Broadcast(FoundUsers);
	}
	else if (Command == TEXT("UPDATE_CHAT")) {
		OnUpdateChatDelegate.Broadcast();
	}
	else if (Command == TEXT("CHAT_HISTORY_RESPONSE")) {
		TArray<FChatMessageData> MessagesList;
		const TArray<TSharedPtr<FJsonValue>>* MessagesArray;

		if (Json->TryGetArrayField(TEXT("messages"), MessagesArray)) {
			for (const auto& Val : *MessagesArray) {
				TSharedPtr<FJsonObject> MsgObj = Val->AsObject();
				if (MsgObj.IsValid()) {
					FChatMessageData MsgData;
					MsgData.ID = MsgObj->GetIntegerField(TEXT("id"));
					MsgData.Sender = MsgObj->GetStringField(TEXT("sender"));
					MsgData.Text = MsgObj->GetStringField(TEXT("text"));
					MsgData.Timestamp = MsgObj->GetStringField(TEXT("timestamp"));

					int32 EditNum = 0, ReadNum = 0;
					MsgData.bIsEdited = MsgObj->TryGetBoolField(TEXT("is_edited"), MsgData.bIsEdited) ? MsgData.bIsEdited : (MsgObj->TryGetNumberField(TEXT("is_edited"), EditNum) && EditNum > 0);
					MsgData.bIsRead = MsgObj->TryGetBoolField(TEXT("is_read"), MsgData.bIsRead) ? MsgData.bIsRead : (MsgObj->TryGetNumberField(TEXT("is_read"), ReadNum) && ReadNum > 0);

					MessagesList.Add(MsgData);
				}
			}
		}
		OnChatHistoryDelegate.Broadcast(MessagesList);
	}
	else if (Command == TEXT("FILE_RECEIVE_START")) {
		FString transferId = Json->GetStringField(TEXT("transfer_id"));
		FString fileName = Json->GetStringField(TEXT("file_name"));
		int32 fileSize = Json->GetIntegerField(TEXT("file_size"));
		NetSubsystem->HandleFileReceiveStart(transferId, fileName, fileSize);
	}
	else if (Command == TEXT("FILE_RECEIVE_CHUNK")) {
		FString transferId = Json->GetStringField(TEXT("transfer_id"));
		FString data = Json->GetStringField(TEXT("data"));
		NetSubsystem->HandleFileReceiveChunk(transferId, data);
	}
	else if (Command == TEXT("FILE_RECEIVE_END")) {
		FString transferId = Json->GetStringField(TEXT("transfer_id"));
		NetSubsystem->HandleFileReceiveEnd(transferId);
	}
}