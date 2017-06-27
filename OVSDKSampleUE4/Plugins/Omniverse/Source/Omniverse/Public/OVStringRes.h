#pragma once

#include "Runtime/Core/Public/Core.h"
#include "OVStringRes.generated.h"

typedef TMap<FString, FString> TStringResMap;

UCLASS()
class OMNIVERSE_API UOVStringRes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadString", Keywords = "LoadString"), Category = "OVSDK")
	static void LoadString(FString pathName = "");

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetString", Keywords = "GetString"), Category = "OVSDK")
	static FString GetString(const FString id);
private:
	static TStringResMap StringResMap;
};

#define OVSTRING(x) FText::FromString(UOVStringRes::GetString(x))
