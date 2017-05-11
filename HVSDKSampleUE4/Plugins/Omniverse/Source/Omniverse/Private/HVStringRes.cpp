#include "OmniversePrivatePCH.h"
#include "HVStringRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>
//#include <Windows.h>

#ifdef LoadString
#undef LoadString
#endif

StringResMap UHVStringRes::_StringResMap;
/*
static FString GetOSLanguage()
{
	FString OSLanguage;

	LCID DefaultLocale = GetUserDefaultUILanguage();

	const int32 MaxLocaleStringLength = 9;
	TCHAR LangBuffer[MaxLocaleStringLength];
	int LangReturn = GetLocaleInfo(DefaultLocale, LOCALE_SISO639LANGNAME, LangBuffer, ARRAY_COUNT(LangBuffer));
	TCHAR CountryBuffer[MaxLocaleStringLength];
	int CountryReturn = GetLocaleInfo(DefaultLocale, LOCALE_SISO3166CTRYNAME, CountryBuffer, ARRAY_COUNT(CountryBuffer));
	
	if (LangReturn != 0 && CountryReturn != 0) {
		OSLanguage = FString::Printf(TEXT("%s-%s"), LangBuffer, CountryBuffer);
	}

	return OSLanguage;
}
*/

static FString GetResFile(FString PathName)
{
	FString OSLanguage = funcGetSysLang(); //GetOSLanguage();
	if (!OSLanguage.IsEmpty()) {
		OSLanguage = "_" + OSLanguage;
	}

	FString ResFile = FPaths::GamePluginsDir() + "/Omniverse/Content/HvSDK/stringres" + OSLanguage + ".xml";
	if (!PathName.IsEmpty())
		ResFile = FPaths::GameDir() + "/Content/HvSDK/" + PathName + "/stringres" + OSLanguage + ".xml";

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ResFile)) {
		OSLanguage.Empty();
	}

	ResFile = FPaths::GamePluginsDir() + "/Omniverse/Content/HvSDK/stringres" + OSLanguage + ".xml";
	if (!PathName.IsEmpty())
		ResFile = FPaths::GameDir() + "/Content/HvSDK/" + PathName + "/stringres" + OSLanguage + ".xml";

	return ResFile;
}

void UHVStringRes::LoadString(FString PathName)
{
	FString ResFile = GetResFile(PathName);

	FXmlFile* XmlFile = new FXmlFile(ResFile);
	if (XmlFile)
	{
		FXmlNode* RootNode = XmlFile->GetRootNode();
		if (RootNode)
		{
			const TArray<FXmlNode*> AssetNodes = RootNode->GetChildrenNodes();
			for (int i = 0; i < AssetNodes.Num(); i++)
			{
				FString sKey = AssetNodes[i]->GetAttribute("id");
				sKey = sKey.Replace(TEXT("\\n"), TEXT("\n"));

				FString sValue = AssetNodes[i]->GetAttribute("str");
				sValue = sValue.Replace(TEXT("\\n"), TEXT("\n"));

				_StringResMap[sKey] = *sValue;
			}
		}
	}
}

FString UHVStringRes::GetString(const FString szID)
{
	StringResMap::iterator i = _StringResMap.find(szID);
	if (i == _StringResMap.end())
		return szID;

	return i->second;
}