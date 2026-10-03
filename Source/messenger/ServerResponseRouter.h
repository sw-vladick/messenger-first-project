#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ServerResponseRouter.generated.h"

USTRUCT(BlueprintType)
struct FChatMessageData
{
	GENERATED_BODY()

	int32 ID;
	FString Sender;
	FString Text;
	FString Timestamp;
	bool bIsEdited;
	bool bIsRead;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthResponse, FString, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchResults, const TArray<FString>&, Users);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatHistory, const TArray<FChatMessageData>&, Messages);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpdateChat);

UCLASS(BlueprintType, Blueprintable)
class MESSENGER_API UServerResponseRouter : public UObject
{
	GENERATED_BODY()
public:
	UServerResponseRouter();
	void ProcessServerResponse(FString Command, FString StatusData);

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnAuthResponse OnLoginResponseDelegate;

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnAuthResponse OnRegisterResponseDelegate;

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnSearchResults OnSearchResultsDelegate;

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnChatHistory OnChatHistoryDelegate;

	UPROPERTY(BlueprintAssignable, Category = "NetworkRouting")
	FOnUpdateChat OnUpdateChatDelegate;
};