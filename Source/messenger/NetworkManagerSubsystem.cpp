// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkManagerSubsystem.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

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
