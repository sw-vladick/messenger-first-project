// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkManagerSubsystem.h"

void UNetworkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ClientSocket = nullptr;
	ResponseRouter = NewObject<UServerResponseRouter>(this);
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
