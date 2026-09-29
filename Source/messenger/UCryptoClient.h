#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
THIRD_PARTY_INCLUDES_START
#define UI UI_ST
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#undef UI
THIRD_PARTY_INCLUDES_END
#include "UCryptoClient.generated.h"

UCLASS()
class MESSENGER_API UCryptoClient : public UObject
{
	GENERATED_BODY()

public:
	UCryptoClient();
	~UCryptoClient();

	bool LoadPublicKey(const FString& FileName);

	void GenerateAESKey();

	TArray<uint8> GetSessionAESKey() const;

	FString GetHandshakePayload();

	FString EncryptAES(const FString& RawString);

	FString DecryptAES(const FString& EncryptedBase64);

private:
	EVP_PKEY* PublicKey;
	TArray<uint8> SessionAESKey;

	FString Base64Encode(const TArray<uint8>& Input);
};