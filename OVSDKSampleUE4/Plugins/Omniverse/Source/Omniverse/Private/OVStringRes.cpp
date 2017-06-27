#include "OmniversePrivatePCH.h"
#include "OVStringRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>

#ifdef LoadString
#undef LoadString
#endif

TStringResMap UOVStringRes::StringResMap;

static FString GetResFile(FString pathName)
{
	FString res_file;
	FString os_lang = funcGetSysLang(); //GetOSLanguage();
	if (!os_lang.IsEmpty()) 
	{
		os_lang = "_" + os_lang;
		res_file = pathName.IsEmpty()
			? FPaths::GamePluginsDir() + "Omniverse/Content/OVSDK/stringres" + os_lang + ".xml"
			: FPaths::GameDir() + "Content/" + pathName + "/stringres" + os_lang + ".xml";

		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*res_file)) {
			return res_file;
		}
	}

	res_file = pathName.IsEmpty()
		? FPaths::GamePluginsDir() + "Omniverse/Content/OVSDK/stringres.xml"
		: FPaths::GameDir() + "Content/" + pathName + "/stringres.xml";

	return res_file;
}

void UOVStringRes::LoadString(FString pathName)
{
	FString res_file = GetResFile(pathName);

	FXmlFile* xml_file = new FXmlFile(res_file);
	if (xml_file)
	{
		FXmlNode* root_node = xml_file->GetRootNode();
		if (root_node)
		{
			const TArray<FXmlNode*> asset_nodes = root_node->GetChildrenNodes();
			for (int i = 0; i < asset_nodes.Num(); i++)
			{
				FString key = asset_nodes[i]->GetAttribute("id");
				key = key.Replace(TEXT("\\n"), TEXT("\n"));

				FString value = asset_nodes[i]->GetAttribute("str");
				value = value.Replace(TEXT("\\n"), TEXT("\n"));

				StringResMap.Add(key) = *value;
			}
		}
	}
}

FString UOVStringRes::GetString(const FString id)
{
	FString *i = StringResMap.Find(id);
	if (i == nullptr) {
		return id;
	}

	return *i;
}