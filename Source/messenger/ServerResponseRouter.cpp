#include "ServerResponseRouter.h"

UServerResponseRouter::UServerResponseRouter() {
	RouteMap.Add(TEXT("LOGIN_RESPONSE"), [this](FString Status) {
		HandleLoginResponse(Status);
		});
	RouteMap.Add(TEXT("REGISTER_RESPONSE"), [this](FString Status) {
		HandleRegisterResponse(Status);
		});
}

void UServerResponseRouter::ProcessServerResponse(FString Command, FString Status) {
	if (RouteMap.Contains(Command)) {
		RouteMap[Command](Status);
	}
	OnServerResponse.Broadcast(Command, Status);
}

void UServerResponseRouter::HandleLoginResponse(FString Status)
{
	UE_LOG(LogTemp, Warning, TEXT("[ROUTER] Логіка логіну оброблена. Статус: %s"), *Status);
}

void UServerResponseRouter::HandleRegisterResponse(FString Status)
{
	UE_LOG(LogTemp, Warning, TEXT("[ROUTER] Логіка реєстрації оброблена. Статус: %s"), *Status);
}
