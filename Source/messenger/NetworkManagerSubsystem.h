// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/ipv4/ipv4address.h"
#include "ServerResponseRouter.h"
#include "DataReceiverRunnable.h"
#include "UCryptoClient.h"
#include "NetworkManagerSubsystem.generated.h"


UCLASS()
class MESSENGER_API UNetworkManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool ConnectToServer(const FString& IPAddress, int32 Port);

	void SendTextMessage(const FString& Message);
	UPROPERTY()
	class UServerResponseRouter* ResponseRouter;

	class DataReceiverRunnable* ReceiverThread;

	class UCryptoClient* CryptoClient;
private:
	FSocket* ClientSocket;
	
};
