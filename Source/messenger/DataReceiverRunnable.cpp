#include "DataReceiverRunnable.h"
#include "ServerResponseRouter.h"
#include "Sockets.h"
#include "HAL/RunnableThread.h"
#include "HAL/PlatformProcess.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DataReceiverRunnable::DataReceiverRunnable(FSocket* InSocket, UServerResponseRouter* InRouter)
{
	Socket = InSocket;
	Router = InRouter;
	bIsRunning = true;
	Thread = FRunnableThread::Create(this, TEXT("NetworkReceiverThread"), 0, TPri_BelowNormal);
}

DataReceiverRunnable::~DataReceiverRunnable()
{
	if (Thread) {
		Thread->Kill(true);
		delete Thread;
	}
}

bool DataReceiverRunnable::Init() { return true; }
void DataReceiverRunnable::Stop() { bIsRunning = false; }
void DataReceiverRunnable::Exit() {}

uint32 DataReceiverRunnable::Run() {
	UE_LOG(LogTemp, Warning, TEXT("[МЕРЕЖА] Фоновий потік запущено. Очікуємо дані від сервера..."));

	while (bIsRunning)
	{
		if (!Socket) break;

		int32 DataSize = 0;
		int32 BytesRead = 0;

		bool bSizeRead = Socket->Recv(reinterpret_cast<uint8*>(&DataSize), sizeof(int32), BytesRead, ESocketReceiveFlags::WaitAll);

		if (bSizeRead && BytesRead == sizeof(int32) && DataSize > 0 && DataSize < 100000) {

			UE_LOG(LogTemp, Warning, TEXT("[МЕРЕЖА] Сервер каже, що зараз прилетить JSON розміром: %d байт"), DataSize);

			TArray<uint8> ReceiveBuffer;
			ReceiveBuffer.SetNumUninitialized(DataSize);
			int32 TotalBytesRead = 0;

			bool bDataRead = Socket->Recv(ReceiveBuffer.GetData(), DataSize, TotalBytesRead, ESocketReceiveFlags::WaitAll);

			if (bDataRead && TotalBytesRead == DataSize) {
				ReceiveBuffer.Add(0);
				FString JsonString = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(ReceiveBuffer.GetData())));

				UE_LOG(LogTemp, Warning, TEXT("[МЕРЕЖА] Успішно отримано JSON: %s"), *JsonString);

				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
				TSharedPtr<FJsonObject> JsonObject;

				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid()) {

					FString Command = JsonObject->GetStringField(TEXT("command"));
					FString StatusData;

					if (JsonObject->HasField(TEXT("payload"))) {
						StatusData = JsonObject->GetStringField(TEXT("payload"));
					}
					else if (JsonObject->HasField(TEXT("status"))) {
						StatusData = JsonObject->GetStringField(TEXT("status"));
					}

					AsyncTask(ENamedThreads::GameThread, [this, Command, StatusData]() {
						if (Router) {
							Router->ProcessServerResponse(Command, StatusData);
						}
						});
				}
			}
		}
		else if (!bSizeRead || BytesRead == 0) {
			FPlatformProcess::Sleep(0.1f);
		}
	}
	return 0;
}