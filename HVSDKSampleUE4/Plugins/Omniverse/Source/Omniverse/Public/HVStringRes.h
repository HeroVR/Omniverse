#pragma once

#include "Runtime/Core/Public/Core.h"
#include "HVStringRes.generated.h"

typedef TMap<FString, FString> TStringResMap;

UCLASS()
class OMNIVERSE_API UHVStringRes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadString", Keywords = "LoadString"), Category = "HVSDK")
	static void LoadString(FString pathName = "");

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetString", Keywords = "GetString"), Category = "HVSDK")
	static FString GetString(const FString id);
private:
	static TStringResMap StringResMap;
};

#define HVSTRING(x) FText::FromString(UHVStringRes::GetString(x))