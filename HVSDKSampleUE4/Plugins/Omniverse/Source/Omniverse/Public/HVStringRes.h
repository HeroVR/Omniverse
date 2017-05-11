#pragma once

#include "Runtime/Core/Public/Core.h"
#include <map>
#include "HVStringRes.generated.h"

typedef std::map<FString, FString> StringResMap;

UCLASS()
class OMNIVERSE_API UHVStringRes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadString", Keywords = "LoadString"), Category = "HVSDK")
	static void LoadString(FString PathName = "");

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetString", Keywords = "GetString"), Category = "HVSDK")
	static FString GetString(const FString szID);
private:
	static StringResMap _StringResMap;
};

#define HVSTRING(x) FText::FromString(UHVStringRes::GetString(x))