#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class FSocket;
class UServerResponseRouter;

class MESSENGER_API DataReceiverRunnable : public FRunnable
{
public:
	DataReceiverRunnable(FSocket* InSocket, UServerResponseRouter* InRouter);

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	~DataReceiverRunnable();
private:
	FSocket* Socket;
	FRunnableThread* Thread;
	FThreadSafeBool bIsRunning;
	UServerResponseRouter* Router;
};
