#include "UCryptoClient.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"

UCryptoClient::UCryptoClient() : PublicKey(nullptr)
{
}

UCryptoClient::~UCryptoClient()
{
	if (PublicKey != nullptr) {
		EVP_PKEY_free(PublicKey);
	}
}

bool UCryptoClient::LoadPublicKey(const FString& FileName)
{
	FString FullPath = FPaths::ProjectDir() + FileName;

	BIO* bio = BIO_new_file(TCHAR_TO_UTF8(*FullPath), "rb");
	if (bio == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("[Crypto] Key file not found: %s"), *FullPath);
		return false;
	}

	PublicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
	BIO_free(bio);

	return PublicKey != nullptr;
}

void UCryptoClient::GenerateAESKey()
{
	SessionAESKey.SetNumUninitialized(32);

	if (RAND_bytes(SessionAESKey.GetData(), 32) != 1) {
		UE_LOG(LogTemp, Error, TEXT("[Crypto] Failed to generate AES key!"));
		SessionAESKey.Empty();
	}
}

TArray<uint8> UCryptoClient::GetSessionAESKey() const
{
	return SessionAESKey;
}

FString UCryptoClient::GetHandshakePayload()
{
	if (PublicKey == nullptr || SessionAESKey.Num() == 0) {
		return TEXT("");
	}

	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(PublicKey, nullptr);
	if (!ctx) return TEXT("");

	if (EVP_PKEY_encrypt_init(ctx) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return TEXT("");
	}

	EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

	size_t outLen = 0;
	if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, SessionAESKey.GetData(), SessionAESKey.Num()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return TEXT("");
	}

	TArray<uint8> EncryptedData;
	EncryptedData.SetNumUninitialized(outLen);

	if (EVP_PKEY_encrypt(ctx, EncryptedData.GetData(), &outLen, SessionAESKey.GetData(), SessionAESKey.Num()) <= 0) {
		EVP_PKEY_CTX_free(ctx);
		return TEXT("");
	}

	EVP_PKEY_CTX_free(ctx);

	EncryptedData.SetNum(outLen);

	return Base64Encode(EncryptedData);
}

FString UCryptoClient::Base64Encode(const TArray<uint8>& Input)
{
	if (Input.Num() == 0) return TEXT("");

	TArray<uint8> Encoded;
	Encoded.SetNumUninitialized(((Input.Num() + 2) / 3) * 4 + 1);

	int len = EVP_EncodeBlock(Encoded.GetData(), Input.GetData(), Input.Num());

	return FString(UTF8_TO_TCHAR(reinterpret_cast<char*>(Encoded.GetData())));
}

FString UCryptoClient::EncryptAES(const FString& RawString)
{
    if (SessionAESKey.Num() != 32) return TEXT("");

    FTCHARToUTF8 Convert(*RawString);
    const uint8* RawData = (const uint8*)Convert.Get();
    int32 RawDataLen = Convert.Length();

    unsigned char iv[16] = { 0 };

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return TEXT("");

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, SessionAESKey.GetData(), iv) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    TArray<uint8> EncryptedData;
    EncryptedData.SetNumUninitialized(RawDataLen + 16);
    int len1 = 0;

    if (EVP_EncryptUpdate(ctx, EncryptedData.GetData(), &len1, RawData, RawDataLen) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    int len2 = 0;
    if (EVP_EncryptFinal_ex(ctx, EncryptedData.GetData() + len1, &len2) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    EVP_CIPHER_CTX_free(ctx);
    EncryptedData.SetNum(len1 + len2);

    return Base64Encode(EncryptedData);
}

FString UCryptoClient::DecryptAES(const FString& EncryptedBase64)
{
    if (SessionAESKey.Num() != 32 || EncryptedBase64.IsEmpty()) return TEXT("");

    TArray<uint8> EncryptedData;
    if (!FBase64::Decode(EncryptedBase64, EncryptedData)) return TEXT("");

    unsigned char iv[16] = { 0 };
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return TEXT("");

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, SessionAESKey.GetData(), iv) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    TArray<uint8> DecryptedData;
    DecryptedData.SetNumUninitialized(EncryptedData.Num() + 16);
    int32 len1 = 0;

    if (EVP_DecryptUpdate(ctx, DecryptedData.GetData(), &len1, EncryptedData.GetData(), EncryptedData.Num()) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    int32 len2 = 0;
    if (EVP_DecryptFinal_ex(ctx, DecryptedData.GetData() + len1, &len2) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return TEXT("");
    }

    EVP_CIPHER_CTX_free(ctx);
    DecryptedData.SetNum(len1 + len2);
    DecryptedData.Add(0);

    return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(DecryptedData.GetData())));
}