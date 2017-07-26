// Fill out your copyright notice in the Description page of Project Settings.
#include "OmniversePrivatePCH.h"
#include "OVDlgBase.h"
#include "OVInterface.h"
#include "OVStringRes.h"
#include "OVRayActor.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

//-------------------------------------------------------------------------------------------------------------------
// Omniverse.Functions.dll
typedef void(*DllCallback)(const char *name, int retCode, const char *result, unsigned resultLen, void *userData);
typedef void(*DllInit)(int gameId, const char *gameKey, const char *params, DllCallback cb, void *userData);
typedef void(*DllDrive)();
typedef IPCDevice* (*DllGetDeviceInfo)();
typedef IPCUser* (*DllGetUserInfo)();
typedef float(*DllGetOmniYawOffset)();
typedef float(*DllGetOmniCoupleRate)();
typedef uint32(*DllGetUserOmniCoupleRate)();
typedef float(*DllSetOmniCoupleRate)(float coupleRate);
typedef float(*DllSetOmniCoupleMode)(bool useCoupleMode);
typedef void(*DllBuy)(const char *itemName, double price, const char *outTradeNo);
typedef void(*DllSendCommand)(unsigned cmd, const char *data, unsigned len);
typedef bool(*DllIsGuest)();
typedef const char* (*DllGetSysLang)();
typedef void(*DllSaveGameData)(const void *data, unsigned len);
typedef void(*DllLoadGameData)();

DllInit funcInit = nullptr;
DllDrive funcDrive = nullptr;
DllGetDeviceInfo funcGetDeviceInfo = nullptr;
DllGetUserInfo funcGetUserInfo = nullptr;
DllGetOmniYawOffset funcGetOmniYawOffset = nullptr;
DllGetOmniCoupleRate funcGetOmniCoupleRate = nullptr;
DllGetUserOmniCoupleRate funcGetUserOmniCoupleRate = nullptr;
DllSetOmniCoupleRate funcSetOmniCoupleRate = nullptr;
DllSetOmniCoupleMode funcSetOmniCoupleMode = nullptr;
DllBuy funcBuy = nullptr;
DllSendCommand funcSendCommand = nullptr;
DllIsGuest funcIsGuest = nullptr;
DllGetSysLang funcGetSysLang = nullptr;
DllSaveGameData funcSaveGameData = nullptr;
DllLoadGameData funcLoadGameData = nullptr;

bool InitDllFunctions(void *dll)
{
#pragma warning(disable:4191)
	funcInit = (DllInit)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllInit"));
	funcDrive = (DllDrive)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllDrive"));
	funcGetDeviceInfo = (DllGetDeviceInfo)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetDeviceInfo"));
	funcGetUserInfo = (DllGetUserInfo)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetUserInfo"));
	funcGetOmniYawOffset = (DllGetOmniYawOffset)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetOmniYawOffset"));
	funcGetOmniCoupleRate = (DllGetOmniCoupleRate)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetOmniCoupleRate"));
	funcGetUserOmniCoupleRate = (DllGetUserOmniCoupleRate)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetUserOmniCoupleRate"));
	funcSetOmniCoupleRate = (DllSetOmniCoupleRate)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllSetOmniCoupleRate"));
	funcSetOmniCoupleMode = (DllSetOmniCoupleMode)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllSetOmniCoupleMode"));
	funcBuy = (DllBuy)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllBuy"));
	funcSendCommand = (DllSendCommand)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllSendCommand"));
	funcIsGuest = (DllIsGuest)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllIsGuest"));
	funcGetSysLang = (DllGetSysLang)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllGetSysLang"));
	funcSaveGameData = (DllSaveGameData)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllSaveGameData"));
	funcLoadGameData = (DllLoadGameData)FWindowsPlatformProcess::GetDllExport(dll, TEXT("DllLoadGameData"));
#pragma warning(default:4191)

	if (NULL == funcInit) {
		UE_LOG(LogTemp, Error, TEXT("OVSDK load Omniverse.Functions.dll interface failed!"));
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("OVSDK load Omniverse.Functions.dll success!"));
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
struct FBuyParam
{
	FString ItemName;
	double Price;
	FString OutTradeNo;
};

struct FResumeGameInfo
{
	int retCode;
	FString retMsg;
	int isAgain;
	double reAmt;
	int reTicket;
};

static bool bHasInit = false;
static bool bInitChecked = false;
static int InitResultCode = 0;
static FString InitResult;
static IPCUser *UserInfo = nullptr;
static IPCDevice *DeviceInfo = nullptr;
static float OmniYawOffset = 0;

static FBuyParam BuyParam;
static FResumeGameInfo ResumeGameInfo;

UOVInterface *UOVInterface::Instance = nullptr;
TArray<AOVDlgBase*> UOVInterface::DlgList;
AOVRayActor* UOVInterface::RayActor = nullptr;
int UOVInterface::RayHandIndex = 1;

void __stdcall UOVInterface::onEventFromDll(const char *name, int retCode, const char *result, unsigned resultLen, void *userData)
{
	UE_LOG(LogTemp, Display, TEXT("OVSDK callback [%s] %s"), UTF8_TO_TCHAR(name), UTF8_TO_TCHAR(result));
	if (strcmp(name, "init") == 0)
	{
		InitResultCode = retCode;
		InitResult = result;
		bHasInit = true;
		bInitChecked = false;

		UserInfo = funcGetUserInfo();
		DeviceInfo = funcGetDeviceInfo();
	}
	else if (strcmp(name, "buy ") == 0)
	{
		UOVInterface::onEventBuyResult(result);
	}
	else if (strcmp(name, "ipc.") == 0)
	{
		if (retCode == 2) {
			UOVInterface::onEventResumeGame(result);
		}
		else if (retCode == 5) {
			UOVInterface::onEventResumeGameResult(result);
		}
		else if (retCode == 9) {
			UOVInterface::onEventSystemMenu(result);
		}
		else if (retCode == 12) {
			UOVInterface::ShowDlgJson(result);
		}
		else if (retCode == 13) {
			UOVInterface::onEventUpdateDlgJsonCmd(result);
		}
		else if (retCode == 16) {
			UOVInterface::onEventCloseDlgJsonCmd(result);
		}
	}
	else if (strcmp(name, "svgd") == 0) {
		onEventSaveGame(retCode, result);
	}
	else if (strcmp(name, "ldgd") == 0) {
		onEventLoadGame(retCode, result, resultLen);
	}
}

UOVInterface::UOVInterface(const FObjectInitializer &objIniter)
	: Super(objIniter), RayRelativeLocation(0, 0, 0), RayRelativeRotation(-60, 0, 0)
{
	UmgDistance = 150;

	RayVisibility = ERayVisibility::Auto;

	bDisableMenuBox = false;
	bDisableResumeGame = false;
}

void UOVInterface::Init(int gameId, FString gameKey, FString param)
{
//#if UE_EDITOR
//	if (HasInitialized())
//		return;
//#endif

	UE_LOG(LogTemp, Display, TEXT("Omniverse init begin..."));

	const char *game_key = TCHAR_TO_UTF8(*gameKey);
	param += ";sdk_type=ue4x;sdk_version=";
	param += OVSDK_VERSION;

	const char *para = TCHAR_TO_UTF8(*param);
	funcInit(gameId, game_key, para, onEventFromDll, nullptr);

	bHasInit = false;
	bInitChecked = false;
	OmniYawOffset = funcGetOmniYawOffset();

	UOVStringRes::LoadString();

	UOVInterface::GetInstance();
}

bool UOVInterface::HasInitialized() {
	return bHasInit;
}

bool UOVInterface::JustInitialized() 
{
	bool ret = false;
	if (bHasInit && !bInitChecked)
	{
		ret = true;
		bInitChecked = true;
	}
	return ret;
}

int UOVInterface::GetInitResultCode() {
	return InitResultCode;
}

FString UOVInterface::GetInitResultMsg() {
	return InitResult;
}

IPCDevice* UOVInterface::GetDeviceInfo() {
	return DeviceInfo;
}

IPCUser* UOVInterface::GetUserInfo() {
	return UserInfo;
}

FString UOVInterface::GetDeviceNo()
{
	return DeviceInfo->sNo;
}

int UOVInterface::GetDeviceId() {
	return DeviceInfo->nId;
}

int UOVInterface::GetShopId() {
	return DeviceInfo->nShopId;
}

FString UOVInterface::GetUserName() {
	return UTF8_TO_TCHAR(UserInfo->sUserName);
}

int UOVInterface::GetUserId() {
	return UserInfo->nUserId;
}

FString UOVInterface::GetUserPhone() {
	return UserInfo->sUserPhone;
}

FString UOVInterface::GetUserEmail() {
	return UserInfo->sUserEmail;
}

float UOVInterface::GetUserBalance() {
	return UserInfo->nUserBalance;
}

float UOVInterface::GetGamePrepareLeft() {
	return UserInfo->nGamePrepareLeft;
}

float UOVInterface::GetGameDurationLeft() {
	return UserInfo->nGameDurationLeft;
}

float UOVInterface::GetGamePrepare() {
	return UserInfo->nGamePrepare;
}

float UOVInterface::GetGameDuration() {
	return UserInfo->nGameDuration;
}

FString UOVInterface::GetBillingMode() {
	return UserInfo->sBillingMode;
}

float UOVInterface::GetOmniCalibrationOffset() {
	return OmniYawOffset;
}

float UOVInterface::GetOmniCoupleRate() 
{
	if (funcGetOmniCoupleRate) {
		return funcGetOmniCoupleRate();
	}
	return 1;
}

uint32 UOVInterface::GetUserOmniCoupleRate()
{
	if (funcGetUserOmniCoupleRate) {
		return funcGetUserOmniCoupleRate();
	}
	return 1;
}

void UOVInterface::SetOmniCoupleRate(float coupleRate) 
{
	if (funcSetOmniCoupleRate) {
		funcSetOmniCoupleRate(coupleRate);
	}	
}

void UOVInterface::SetOmniCoupleMode(bool useCoupleMode)
{
	if (funcSetOmniCoupleMode) {
		funcSetOmniCoupleMode(useCoupleMode);
	}
}

void UOVInterface::Buy(FString itemName, float price, FString outTradeNo)
{
	if (IsGuest() || GetUserId() == 0) {
		MsgBox(OVSTRING("Notice"), OVSTRING("BuyGuest"), nullptr, EMsgBoxFlag::Ok);
		return;
	}

	BuyParam.ItemName = itemName;
	BuyParam.Price = price;
	BuyParam.OutTradeNo = outTradeNo;

	UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"),	FText::Format(OVSTRING("BuyNotice")
			, FFormatArgumentValue(FText::FromString(BuyParam.ItemName)), FFormatArgumentValue(BuyParam.Price)), &UOVInterface::BuyMsgBoxCallBack);
}

void UOVInterface::SendCommand(int nCmd, FString sData, int nLen)
{
	if (funcSendCommand) {
		funcSendCommand(nCmd, TCHAR_TO_ANSI(*sData), nLen);
	}
}

bool UOVInterface::IsGuest()
{
	if (funcIsGuest) {
		return funcIsGuest();
	}
	return false;
}

void UOVInterface::ConfirmPlayGame()
{
	SendCommand(6, "", 0);
}

//copy from Engine\Source\Runtime\Engine\Private\GameplayStatics.cpp 
//			bool UGameplayStatics::SaveGameToSlot(USaveGame* SaveGameObject, const FString& SlotName, const int32 UserIndex)
static const int UE4_SAVEGAME_FILE_TYPE_TAG = 0x53415647;		// "sAvG"
struct FSaveGameFileVersion
{
	enum Type
	{
		InitialVersion = 1,
		// serializing custom versions into the savegame data to handle that type of versioning
		AddedCustomVersions = 2,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};
static void SaveGameDataToBytes(USaveGame *SaveGameObject, TArray<uint8> &ObjectBytes)
{
	FMemoryWriter MemoryWriter(ObjectBytes, true);

	// write file type tag. identifies this file type and indicates it's using proper versioning
	// since older UE4 versions did not version this data.
	int32 FileTypeTag = UE4_SAVEGAME_FILE_TYPE_TAG;
	MemoryWriter << FileTypeTag;

	// Write version for this file format
	int32 SavegameFileVersion = FSaveGameFileVersion::LatestVersion;
	MemoryWriter << SavegameFileVersion;

	// Write out engine and UE4 version information
	int32 PackageFileUE4Version = GPackageFileUE4Version;
	MemoryWriter << PackageFileUE4Version;
	FEngineVersion SavedEngineVersion = FEngineVersion::Current();
	MemoryWriter << SavedEngineVersion;

	// Write out custom version data
	ECustomVersionSerializationFormat::Type const CustomVersionFormat = ECustomVersionSerializationFormat::Latest;
	int32 CustomVersionFormatInt = static_cast<int32>(CustomVersionFormat);
	MemoryWriter << CustomVersionFormatInt;
	FCustomVersionContainer CustomVersions = FCustomVersionContainer::GetRegistered();
	CustomVersions.Serialize(MemoryWriter, CustomVersionFormat);

	// Write the class name so we know what class to load to
	FString SaveGameClassName = SaveGameObject->GetClass()->GetName();
	MemoryWriter << SaveGameClassName;

	// Then save the object state, replacing object refs and names with strings
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
	SaveGameObject->Serialize(Ar);
}

static USaveGame* LoadGameDataFromBytes(const TArray<uint8> &ObjectBytes)
{
	USaveGame* OutSaveGameObject = NULL;
	FMemoryReader MemoryReader(ObjectBytes, true);

	int32 FileTypeTag;
	MemoryReader << FileTypeTag;

	int32 SavegameFileVersion;
	if (FileTypeTag != UE4_SAVEGAME_FILE_TYPE_TAG)
	{
		// this is an old saved game, back up the file pointer to the beginning and assume version 1
		MemoryReader.Seek(0);
		SavegameFileVersion = FSaveGameFileVersion::InitialVersion;

		// Note for 4.8 and beyond: if you get a crash loading a pre-4.8 version of your savegame file and 
		// you don't want to delete it, try uncommenting these lines and changing them to use the version 
		// information from your previous build. Then load and resave your savegame file.
		//MemoryReader.SetUE4Ver(MyPreviousUE4Version);				// @see GPackageFileUE4Version
		//MemoryReader.SetEngineVer(MyPreviousEngineVersion);		// @see FEngineVersion::Current()
	}
	else
	{
		// Read version for this file format
		MemoryReader << SavegameFileVersion;

		// Read engine and UE4 version information
		int32 SavedUE4Version;
		MemoryReader << SavedUE4Version;

		FEngineVersion SavedEngineVersion;
		MemoryReader << SavedEngineVersion;

		MemoryReader.SetUE4Ver(SavedUE4Version);
		MemoryReader.SetEngineVer(SavedEngineVersion);

		if (SavegameFileVersion >= FSaveGameFileVersion::AddedCustomVersions)
		{
			int32 CustomVersionFormat;
			MemoryReader << CustomVersionFormat;

			FCustomVersionContainer CustomVersions;
			CustomVersions.Serialize(MemoryReader, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));
			MemoryReader.SetCustomVersions(CustomVersions);
		}
	}

	// Get the class name
	FString SaveGameClassName;
	MemoryReader << SaveGameClassName;

	// Try and find it, and failing that, load it
	UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveGameClassName);
	if (SaveGameClass == NULL)
	{
		SaveGameClass = LoadObject<UClass>(NULL, *SaveGameClassName);
	}

	// If we have a class, try and load it.
	if (SaveGameClass != NULL)
	{
		OutSaveGameObject = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);

		FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
		OutSaveGameObject->Serialize(Ar);
	}

	return OutSaveGameObject;
}

bool UOVInterface::SaveGameData(USaveGame *data)
{
	if (IsGuest() || nullptr == funcSaveGameData) {
		MsgBox(FText::FromString("Error"), FText::FromString("Can't save game for a guest player."), nullptr, EMsgBoxFlag::Ok);
		return false;
	}

	TArray<uint8> bytes;
	SaveGameDataToBytes(data, bytes);
	funcSaveGameData(bytes.GetData(), bytes.Num());
	return true;
}

bool UOVInterface::LoadGameData()
{
	if (IsGuest() || nullptr == funcLoadGameData) {
		return false;
	}
	
	funcLoadGameData();
	return true;
}

void UOVInterface::onEventSaveGame(int retCode, const char *result)
{
	UOVInterface::GetInstance()->OnSaveGameData.Broadcast(retCode, result);
}

void UOVInterface::onEventLoadGame(int retCode, const char *result, unsigned resultLen)
{
	USaveGame *data = nullptr;
	FString retMsg = "";
	if (0 == retCode)
	{
		TArray<uint8> bytes;
		bytes.AddDefaulted(resultLen);
		memcpy(bytes.GetData(), result, resultLen);
		data = LoadGameDataFromBytes(bytes);
	}
 	else {
		retMsg = UTF8_TO_TCHAR(result);
	}

	UOVInterface::GetInstance()->OnLoadGameData.Broadcast(retCode, retMsg, data);
}

UOVInterface* UOVInterface::GetInstance()
{
	if (Instance == nullptr) 
	{
		Instance = NewObject<UOVInterface>();
		Instance->AddToRoot();
	}
	return Instance;
}

AOVDlgBase *UOVInterface::MsgBoxBP(FText title, FText content, EMsgBoxFlag::Flag flag)
{
	return MsgBox(title, content, nullptr, flag);
}

AOVDlgBase *UOVInterface::MsgBox(FText title, FText content, OVSDKMsgBoxCallback Func, EMsgBoxFlag::Flag flag)
{
	AOVMsgBox *Box = nullptr;
	UWorld *world = GetGameWorld();
	if (world)
	{
		Box = world->SpawnActor<AOVMsgBox>(AOVMsgBox::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Box) {
			Box->Init(title, content, Func, flag);
		}
	}

	return Box;
}

AOVDlgBase *UOVInterface::ShowDlgJson(const FString &jsonFilePrefix)
{
	UWorld *world = GetGameWorld();
	if (nullptr == world) {
		return nullptr;
	}

	AOVDlgJson *dlg = world->SpawnActor<AOVDlgJson>(AOVDlgJson::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (dlg) {
		dlg->LoadJson(TCHAR_TO_ANSI(*jsonFilePrefix));
	}
	return dlg;	
}

AOVDlgBase* UOVInterface::ShowDlgBP(TSubclassOf<UUserWidget> uiAsset, FVector loc, FRotator rot)
{
	UWorld *world = GetGameWorld();
	if (nullptr == world) {
		return nullptr;
	}

	AOVDlgBase *dlg = world->SpawnActor<AOVDlgBase>(AOVDlgBase::StaticClass(), loc, rot);
	if (dlg) {
		dlg->SetWidgetClass(uiAsset);
	}
	return dlg;
}

AOVDlgBase *UOVInterface::ShowDlgPrompt(FText text, FVector loc, FRotator rot, bool alwaysOnCamera, bool billboard, float duration, float fadeout)
{
	UWorld *world = GetGameWorld();
	if (nullptr == world) {
		return nullptr;
	}

	AOVDlgBase *dlg = nullptr;
	UClass *clz = StaticLoadClass(UUserWidget::StaticClass(), world, TEXT("Class'/Omniverse/OVSDK/Prompt.Prompt_C'"));
	if (clz)
	{
		dlg = world->SpawnActor<AOVDlgBase>(AOVDlgBase::StaticClass(), loc, rot);
		if (dlg)
		{
			dlg->SetAttribute(false, alwaysOnCamera, billboard, duration, fadeout);

			dlg->SetWidgetClass(clz);
			dlg->InitWidget("content", text);
		}
	}

	return dlg;
}

void UOVInterface::UpdateDlgJsonWidgetCmd(const FString &DlgJsonName, const FString &widgetName, const FString &cmd)
{
	bool find_succ = false;
	FString name;
	for (int i = 0; i < DlgList.Num(); ++i)
	{
		AOVDlgJson *dlg = Cast<AOVDlgJson>(DlgList[i]);
		if (dlg != nullptr && dlg->DlgJsonName == DlgJsonName)
		{
			dlg->UpdateCmd(widgetName, cmd);
			find_succ = true;
			name = dlg->DlgJsonName;
		}
	}
	if (!find_succ)
	{
		FString tmp = DlgList.Num() > 0 ? DlgJsonName : "";
		UOVInterface::SendCommand(15, tmp, tmp.Len());
	}
}

void UOVInterface::onEventUpdateDlgJsonCmd(const char *result)
{
	FString param = UTF8_TO_TCHAR(result);
	FString dlg_name, widget_name, cmd;

	int32 sep = 0, sep2 = 0;
	if (param.FindChar(' ', sep))
	{
		dlg_name = param.Mid(0, sep);
		sep2 = param.Find(TEXT(" "), ESearchCase::CaseSensitive, ESearchDir::FromStart, sep + 1);
		if (sep2 > 0)
		{
			widget_name = param.Mid(sep + 1, sep2 - sep - 1);
			cmd = param.Mid(sep2 + 1);
		}
	}

	if (!cmd.IsEmpty())	{
		UpdateDlgJsonWidgetCmd(dlg_name, widget_name, cmd);
	}
	
}

void UOVInterface::onEventCloseDlgJsonCmd(const char *result)
{
	FString name;
	if ((NULL != result) && (0 != result[0])) {
		name = result;
	}
	TArray<AOVDlgJson *> dlgs;
	for (int i = (DlgList.Num() - 1); i >= 0; --i)
	{
		AOVDlgJson *dlg = Cast<AOVDlgJson>(DlgList[i]);
		if (NULL != dlg) {
			dlgs.Add(dlg);
		}
	}
	for (int i = (dlgs.Num() - 1); i >= 0; --i) {
		dlgs[i]->TryClose(name);
	}
}

UWorld* UOVInterface::GetGameWorld()
{
	const TIndirectArray<FWorldContext>& worlds = GEngine->GetWorldContexts();
	for (int i = 0; i < worlds.Num(); ++i)
	{
		const FWorldContext &wc = worlds[i];
		if ((wc.WorldType == EWorldType::Game || wc.WorldType == EWorldType::PIE || wc.WorldType == EWorldType::GamePreview)
			&& wc.World()->GetFirstPlayerController())
		{
			return wc.World();
		}
	}

	//UE_LOG(LogTemp, Error, TEXT("UOVInterface::GetGameWorld failed."));
	return nullptr;
}

void UOVInterface::Tick(float DeltaTime)
{
	if (nullptr == RayActor)
	{
		UWorld *world = GetGameWorld();
		if (world)
		{
			RayActor = world->SpawnActor<AOVRayActor>(AOVRayActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			RayActor->SetRayHand(RayHandIndex);
		}
	}	
	
	if (RayActor)
	{
		funcDrive();
	}
}

void UOVInterface::BeginPlayDlgBase(AOVDlgBase *dlg)
{
	dlg->DlgIndex = DlgList.Num();

	DlgList.Add(dlg);
}

void UOVInterface::EndPlayDlgBase(AOVDlgBase *dlg)
{
	if (DlgList.Remove(dlg) >= 0)
	{
		for (int i = 0; i < DlgList.Num(); ++i) {
			DlgList[i]->DlgIndex = i;
		}
	}	
}

void UOVInterface::OnRayActorDestroyed()
{
	RayActor = nullptr;
}

void UOVInterface::BeginPlay()
{
	EndPlay();

	UWorld *world = GetGameWorld();
	if (world)
	{
		RayActor = world->SpawnActor<AOVRayActor>(AOVRayActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		RayActor->SetRayHand(RayHandIndex);
	}
}

void UOVInterface::EndPlay()
{
	if (RayActor) 
	{
		UWorld *world = RayActor->GetWorld();
		if (world)
		{
			while (DlgList.Num()) {
				world->DestroyActor(DlgList[0]);
			}

			world->DestroyActor(RayActor);
		}

		RayActor = nullptr;
	}	

	DlgList.Empty();	
}

AOVRayActor *UOVInterface::GetActor() {
	return RayActor;
}

void UOVInterface::onEventBuyResult(const char *result)
{
	TSharedPtr<FJsonValue> json_parsed;
	TSharedRef< TJsonReader<TCHAR> > json_reader = TJsonReaderFactory<TCHAR>::Create(UTF8_TO_TCHAR(result));
	if (FJsonSerializer::Deserialize(json_reader, json_parsed))
	{
		int ret = json_parsed->AsObject()->GetIntegerField("retCode");
		FString msg = json_parsed->AsObject()->GetStringField("retMsg");
		FString inTradeNo = json_parsed->AsObject()->GetStringField("intradeno");
		if (ret != 0)
		{
			UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"),
				FText::Format(OVSTRING("BuyFaild")
					, FFormatArgumentValue(FText::FromString(msg))), nullptr, EMsgBoxFlag::Ok);
		}
		else 
		{
			UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"),
				FText::Format(OVSTRING("BuySuccess")
					, FFormatArgumentValue(FText::FromString(inTradeNo))), nullptr, EMsgBoxFlag::Ok);

			UWorld *world = GetGameWorld();
			if (world && UOVInterface::GetInstance()->GetActor()) {
				UOVInterface::GetInstance()->OnBuyResult.Broadcast(BuyParam.ItemName, BuyParam.OutTradeNo, inTradeNo, msg);
			}
		}
	}
}
void UOVInterface::BuyMsgBoxCallBack(int32 btnIndex)
{
	if (1 == btnIndex)
	{
		if (funcBuy) {
			funcBuy(TCHAR_TO_ANSI(*BuyParam.ItemName), BuyParam.Price, TCHAR_TO_ANSI(*BuyParam.OutTradeNo));
		}
	}
}

void UOVInterface::ResumeGameMsgBoxCallBack(int32 btnIndex)
{
	if (1 == btnIndex)
	{
		if (ResumeGameInfo.isAgain != 0) {
			SendCommand(3, "", 0);
		}
	}
}

void UOVInterface::onEventResumeGame(const char *result)
{
	if (GetInstance()->bDisableMenuBox) {
		return;
	}

	TSharedPtr<FJsonValue> json_parsed;
	TSharedRef< TJsonReader<TCHAR> > json_reader = TJsonReaderFactory<TCHAR>::Create(UTF8_TO_TCHAR(result));
	if (FJsonSerializer::Deserialize(json_reader, json_parsed))
	{
		ResumeGameInfo.retCode = json_parsed->AsObject()->GetIntegerField("retCode");
		ResumeGameInfo.retMsg = json_parsed->AsObject()->GetStringField("retMsg");
		ResumeGameInfo.isAgain = json_parsed->AsObject()->GetIntegerField("isAgain");
		ResumeGameInfo.reAmt = json_parsed->AsObject()->GetNumberField("reAmt");
		ResumeGameInfo.reTicket = json_parsed->AsObject()->GetIntegerField("reTicket");
		
		UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"),
			   FText::Format(OVSTRING("ResumeNotice")
				   , FFormatArgumentValue(ResumeGameInfo.reAmt), FFormatArgumentValue(ResumeGameInfo.reTicket)), &UOVInterface::ResumeGameMsgBoxCallBack);

		UWorld *world = GetGameWorld();
		if (world && UOVInterface::GetInstance()->GetActor()) {
			UOVInterface::GetInstance()->OnResumeGame.Broadcast(ResumeGameInfo.reAmt, ResumeGameInfo.reTicket);
		}
	}
}

void UOVInterface::onEventResumeGameResult(const char *result)
{
	if (strlen(result) > 0) {
		UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"), OVSTRING("ResumeFaild"), nullptr, EMsgBoxFlag::Ok);
	}
	else {
		UOVInterface::GetInstance()->MsgBox(OVSTRING("Notice"), OVSTRING("ResumeSuccess"), nullptr, EMsgBoxFlag::Ok);
	}
}

void UOVInterface::onEventSystemMenu(const char *result)
{
	ShowDlgJson("systemmenu");
}

void UOVInterface::SetRayHand(int i)
{
	UOVInterface *thiz = GetInstance();
	thiz->RayHandIndex = i;

	if (thiz->RayActor) {
		thiz->RayActor->SetRayHand(i);
	}
}
