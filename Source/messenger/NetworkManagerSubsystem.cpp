#include "NetworkManagerSubsystem.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Base64.h"

void UNetworkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ClientSocket = nullptr;
	ResponseRouter = NewObject<UServerResponseRouter>(this);

	CryptoClient = NewObject<UCryptoClient>(this);
	if (!CryptoClient->LoadPublicKey(TEXT("public_key.pem"))) {
		UE_LOG(LogTemp, Error, TEXT("ERROR: Failed to load public key!"));
	}
}

void UNetworkManagerSubsystem::Deinitialize()
{
	if (ClientSocket) {
		ClientSocket->Close();
	}

	if (ReceiverThread) {
		ReceiverThread->Stop();
		delete ReceiverThread;
		ReceiverThread = nullptr;
	}

	if (ClientSocket) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}

	Super::Deinitialize();
}

bool UNetworkManagerSubsystem::ConnectToServer(const FString& IPAddress, int32 Port)
{
	if (ClientSocket) return true;

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("ClientSocket"), false);

	FIPv4Address ip;
	FIPv4Address::Parse(IPAddress, ip);

	TSharedRef<FInternetAddr> ServerAddr = SocketSubsystem->CreateInternetAddr();
	ServerAddr->SetIp(ip.Value);
	ServerAddr->SetPort(Port);

	bool bIsConnected = ClientSocket->Connect(*ServerAddr);
	if (bIsConnected) {
		ReceiverThread = new DataReceiverRunnable(ClientSocket, ResponseRouter);

		CryptoClient->GenerateAESKey();

		FString EncryptedKeyBase64 = CryptoClient->GetHandshakePayload();

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		JsonObject->SetStringField(TEXT("Command"), TEXT("HANDSHAKE"));
		JsonObject->SetStringField(TEXT("key"), EncryptedKeyBase64);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		SendTextMessage(OutputString);
	}
	return bIsConnected;
}

void UNetworkManagerSubsystem::SendTextMessage(const FString& Message)
{
	if (!ClientSocket) return;
	
	FTCHARToUTF8 Convert(*Message);
	int32 MessageLenght = Convert.Length();
	int32 BytesSent = 0;

	ClientSocket->Send((uint8*)&MessageLenght, 4, BytesSent);

	ClientSocket->Send((uint8*)Convert.Get(), Convert.Length(), BytesSent);
}

void UNetworkManagerSubsystem::SendEncryptedCommand(const FString& CommandName, TSharedPtr<FJsonObject> InnerData)
{
	TSharedPtr<FJsonObject> EnvelopeJson = MakeShareable(new FJsonObject());
	EnvelopeJson->SetStringField(TEXT("Command"), CommandName);

	if (InnerData.IsValid() && CryptoClient) {
		FString InnerPayload;
		TSharedRef<TJsonWriter<>> InnerWriter = TJsonWriterFactory<>::Create(&InnerPayload);
		FJsonSerializer::Serialize(InnerData.ToSharedRef(), InnerWriter);

		FString EncryptedBase64 = CryptoClient->EncryptAES(InnerPayload);
		EnvelopeJson->SetStringField(TEXT("payload"), EncryptedBase64);
	}

	FString FinalString;
	TSharedRef<TJsonWriter<>> EnvWriter = TJsonWriterFactory<>::Create(&FinalString);
	FJsonSerializer::Serialize(EnvelopeJson.ToSharedRef(), EnvWriter);

	SendTextMessage(FinalString);
}

void UNetworkManagerSubsystem::SendFileToServer(const FString& FilePath, const FString& TargetUser)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*FilePath);

	if (!FileHandle) {
		UE_LOG(LogTemp, Error, TEXT("[FILE] Failed to open file!"));
		return;
	}

	int64 FileSize = FileHandle->Size();
	int64 BytesRead = 0;
	int32 ChunkSize = 16 * 1024;

	FString TransferID = FGuid::NewGuid().ToString();
	FString FileName = FPaths::GetCleanFilename(FilePath);

	TSharedPtr<FJsonObject> StartJson = MakeShareable(new FJsonObject());
	StartJson->SetStringField(TEXT("transfer_id"), TransferID);
	StartJson->SetStringField(TEXT("file_name"), FileName);
	StartJson->SetNumberField(TEXT("file_size"), FileSize);
	StartJson->SetStringField(TEXT("target_user"), TargetUser);
	SendEncryptedCommand(TEXT("FILE_START"), StartJson);

	TArray<uint8> ChunkBuffer;
	ChunkBuffer.SetNumUninitialized(ChunkSize);
	int32 ChunkIndex = 0;

	while (BytesRead < FileSize) {
		int64 BytesToRead = FMath::Min((int64)ChunkSize, FileSize - BytesRead);
		FileHandle->Read(ChunkBuffer.GetData(), BytesToRead);

		TArray<uint8> ActualData(ChunkBuffer.GetData(), BytesToRead);
		FString Base64Chunk = FBase64::Encode(ActualData);

		TSharedPtr<FJsonObject> ChunkJson = MakeShareable(new FJsonObject());
		ChunkJson->SetStringField(TEXT("transfer_id"), TransferID);
		ChunkJson->SetNumberField(TEXT("chunk_index"), ChunkIndex);
		ChunkJson->SetStringField(TEXT("data"), Base64Chunk);

		SendEncryptedCommand(TEXT("FILE_CHUNK"), ChunkJson);

		BytesRead += BytesToRead;
		ChunkIndex++;
	}
	delete FileHandle;

	TSharedPtr<FJsonObject> EndJson = MakeShareable(new FJsonObject());
	EndJson->SetStringField(TEXT("transfer_id"), TransferID);
	EndJson->SetStringField(TEXT("target_user"), TargetUser);
	SendEncryptedCommand(TEXT("FILE_END"), EndJson);

	UE_LOG(LogTemp, Warning, TEXT("[FILE] Successfully sent %d chunks of file: %s"), ChunkIndex, *FileName);
}

void UNetworkManagerSubsystem::HandleFileReceiveStart(const FString& TransferID, const FString& FileName, int32 FileSize)
{
	FIncomingFileSession Session;
	Session.FileName = FileName;
	Session.ExpectedSize = FileSize;
	IncomingFiles.Add(TransferID, Session);

}

void UNetworkManagerSubsystem::HandleFileReceiveChunk(const FString& TransferID, const FString& Base64Data)
{
	if (!IncomingFiles.Contains(TransferID)) return;

	TArray<uint8> DecodedBytes;
	if (FBase64::Decode(Base64Data, DecodedBytes)) {
		IncomingFiles[TransferID].Buffer.Append(DecodedBytes);
	}
}

void UNetworkManagerSubsystem::HandleFileReceiveEnd(const FString& TransferID)
{
	if (!IncomingFiles.Contains(TransferID)) return;

	FIncomingFileSession& Session = IncomingFiles[TransferID];

	FString SaveDirectory = FPaths::ProjectSavedDir() / TEXT("Downloads");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*SaveDirectory)) {
		PlatformFile.CreateDirectoryTree(*SaveDirectory);
	}

	FString FullPath = SaveDirectory / Session.FileName;

	if (FFileHelper::SaveArrayToFile(Session.Buffer, *FullPath)) {
		UE_LOG(LogTemp, Warning, TEXT("[FILE] success: %s"), *FullPath);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("[FILE] failed: %s"), *FullPath);
	}

	IncomingFiles.Remove(TransferID);
}