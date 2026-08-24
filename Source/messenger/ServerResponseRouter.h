#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ServerResponseRouter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServerResponse, FString, Command, FString, Status);

UCLASS(BlueprintType, Blueprintable)
class MESSENGER_API UServerResponseRouter : public UObject
{
	GENERATED_BODY()
public:
	UServerResponseRouter();

	void ProcessServerResponse(FString Command, FString Status);

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnServerResponse OnServerResponse;

private:
	TMap<FString, TFunction<void(FString)>> RouteMap;

	void HandleLoginResponse(FString Status);
	void HandleRegisterResponse(FString Status);
};
