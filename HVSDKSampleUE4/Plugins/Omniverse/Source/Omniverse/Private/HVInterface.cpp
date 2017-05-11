// Fill out your copyright notice in the Description page of Project Settings.
#include "OmniversePrivatePCH.h"
#include "HVMenuBox.h"
#include "HVDlgBase.h"
#include "HVInterface.h"
#include "HVStringRes.h"
#include "HVRayActor.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"


HvInit funcInit = nullptr;
HvDrive funcDrive = nullptr;
HvGetDeviceInfo funcGetDeviceInfo = nullptr;
HvGetUserInfo funcGetUserInfo = nullptr;
HvGetOmniYawOffset funcGetOmniYawOffset = nullptr;
HvBuy funcBuy = nullptr;
HvSendCommand funcSendCommand = nullptr;
HvIsGuest funcIsGuest = nullptr;
HvGetSysLang funcGetSysLang = nullptr;

static bool bHasInit = false;
static bool bInitChecked = false;
static FString InitResult;
static IPCUser *UserInfo = nullptr;
static IPCDevice *DeviceInfo = nullptr;
static float OmniYawOffset = 0;

UHvInterface *UHvInterface::Instance = nullptr;
HVBUY_CALLBACK UHvInterface::BuyCallBack = nullptr;
FBuyParam UHvInterface::BuyParam;
FResumeGameInfo UHvInterface::ResumeGameInfo;
TArray<AHVDlgBase*> UHvInterface::DlgList;
AHVRayActor* UHvInterface::RayActor = nullptr;
int UHvInterface::RayHandIndex = 1;

void __stdcall UHvInterface::dllcallback(const char *sName, const char *sRet, unsigned nRetLen, void *pUserData)
{
	UE_LOG(LogTemp, Display, TEXT("HVSDK callback [%s] %s"), UTF8_TO_TCHAR(sName), UTF8_TO_TCHAR(sRet));
	if (strcmp(sName, "init") == 0)
	{
		InitResult = sRet;
		bHasInit = true;
		bInitChecked = false;

		UserInfo = funcGetUserInfo();
		DeviceInfo = funcGetDeviceInfo();
	}
	else if (strcmp(sName, "buy ") == 0)
	{
		UHvInterface::onEventBuyResult(sRet);
	}
	else if (strncmp(sName, "ipc.", 4) == 0)
	{
		if (sName[4] == '2')
		{
			UHvInterface::onEventResumeGame(sRet);
		}
		else if (sName[4] == '5')
		{
			UHvInterface::onEventResumeGameResult(sRet);
		}
		else if (sName[4] == '9')
		{
			UHvInterface::onEventSystemMenu(sRet);
		}
	}
}

UHvInterface::UHvInterface(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	UmgDistance = 150;

	RayVisibility = ERayVisibility::Auto;

	bDisableMenuBox = false;
	bDisableResumeGame = false;
}

void UHvInterface::init(int nGameId, FString sGameKey, FString sParam)
{
//#if UE_EDITOR
//	if (hasInit())
//		return;
//#endif

	UE_LOG(LogTemp, Display, TEXT("Omniverse init begin..."));

	const char *game_key = TCHAR_TO_UTF8(*sGameKey);
	sParam += ";hvsdk_type=ue4x;hvsdk_version=";
	sParam += HVSDK_VERSION;

	const char *param = TCHAR_TO_UTF8(*sParam);
	funcInit(nGameId, game_key, param, dllcallback, nullptr);

	bHasInit = false;
	bInitChecked = false;
	OmniYawOffset = funcGetOmniYawOffset();

	UHVStringRes::LoadString();

	UHvInterface::GetInstance();
}

bool UHvInterface::hasInit() {
	return bHasInit;
}

bool UHvInterface::isJustInitDone() 
{
	bool ret = false;
	if (bHasInit && !bInitChecked)
	{
		ret = true;
		bInitChecked = true;
	}
	return ret;
}

FString UHvInterface::getInitRet() {
	return InitResult;
}

void UHvInterface::drive() 
{
}

IPCDevice* UHvInterface::getDeviceInfo() {
	return funcGetDeviceInfo();
}

IPCUser* UHvInterface::getUserInfo() {
	return funcGetUserInfo();
}

FString UHvInterface::getDeviceNo()
{
	IPCDevice *info = funcGetDeviceInfo();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, info->sNo);
	return info->sNo;
}

int UHvInterface::getDeviceId() {
	return DeviceInfo->nId;
}

int UHvInterface::getShopId() {
	return DeviceInfo->nShopId;
}

FString UHvInterface::getUserName() {
	return UTF8_TO_TCHAR(UserInfo->sName);
}

int UHvInterface::getUserId() {
	return UserInfo->nId;
}

FString UHvInterface::getUserPhone() {
	return UserInfo->sPhone;
}

FString UHvInterface::getUserEmail() {
	return UserInfo->sEmail;
}

float UHvInterface::getUserWallet() {
	return UserInfo->nWallet;
}

float UHvInterface::getGamePrepareLeft() {
	return UserInfo->nGamePrepareLeft;
}

float UHvInterface::getGameDurationLeft() {
	return UserInfo->nGameDurationLeft;
}

float UHvInterface::getGamePrepare() {
	return UserInfo->nGamePrepare;
}

float UHvInterface::getGameDuration() {
	return UserInfo->nGameDuration;
}

FString UHvInterface::getBillingMode() {
	return UserInfo->sBillingMode;
}

float UHvInterface::getOmniYawOffset() {
	return OmniYawOffset;
}


void UHvInterface::RegisterBuyCallBack(HVBUY_CALLBACK buyCallBack) {
	BuyCallBack = buyCallBack;
}

void UHvInterface::buy(FString sItem, float nPrice, FString sOutTradeNo)
{
	if (isGuest() || getUserId() == 0) {
		MsgBox(HVSTRING("Notice"), HVSTRING("BuyGuest"), nullptr, EMsgBoxFlag::Ok);
		return;
	}

	BuyParam.sItem = sItem;
	BuyParam.nPrice = nPrice;
	BuyParam.sOutTradeNo = sOutTradeNo;

	UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"),
		FText::Format(HVSTRING("BuyNotice")
			, FFormatArgumentValue(FText::FromString(BuyParam.sItem)), FFormatArgumentValue(BuyParam.nPrice)), &UHvInterface::BuyMsgBoxCallBack);
}

void UHvInterface::sendCommand(int nCmd, FString sData, int nLen)
{
	if (funcSendCommand) {
		funcSendCommand(nCmd, TCHAR_TO_ANSI(*sData), nLen);
	}
}

bool UHvInterface::isGuest()
{
	if (funcIsGuest) {
		return funcIsGuest();
	}
	return false;
}

void UHvInterface::confirmPlayGame()
{
	sendCommand(6, "", 0);
}

UHvInterface* UHvInterface::GetInstance()
{
	if (Instance == nullptr) 
	{
		Instance = NewObject<UHvInterface>();
		Instance->AddToRoot();
	}
	return Instance;
}

AHVDlgBase *UHvInterface::MsgBoxBP(FText Title, FText Content, EMsgBoxFlag::Flag Flag)
{
	return MsgBox(Title, Content, nullptr, Flag);
}

AHVDlgBase *UHvInterface::MsgBox(FText Title, FText Content, HVMSGBOX_CALLBACK Func, EMsgBoxFlag::Flag Flag)
{
	AHVMsgBox *Box = nullptr;
	UWorld *world = GetGameWorld();
	if (world)
	{
		Box = world->SpawnActor<AHVMsgBox>(AHVMsgBox::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Box) {
			Box->Init(Title, Content, Func, Flag);
		}
	}

	return Box;
}

UWorld* UHvInterface::GetGameWorld()
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

	//UE_LOG(LogTemp, Error, TEXT("UHvInterface::GetGameWorld failed."));
	return nullptr;
}

void UHvInterface::Tick(float DeltaTime)
{
	if (nullptr == RayActor)
	{
		UWorld *world = GetGameWorld();
		if (world)
		{
			RayActor = world->SpawnActor<AHVRayActor>(AHVRayActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			RayActor->SetRayHand(RayHandIndex);
		}
	}	
	
	if (RayActor)
	{
		funcDrive();
	}
}

void UHvInterface::BeginPlayDlgBase(AHVDlgBase *Dlg)
{
	Dlg->DlgIndex = DlgList.Num();

	DlgList.Add(Dlg);
}

void UHvInterface::EndPlayDlgBase(AHVDlgBase *Dlg)
{
	DlgList.Remove(Dlg);

	for (int i = 0; i < DlgList.Num(); ++i) {
		DlgList[i]->DlgIndex = i;
	}
}

void UHvInterface::OnRayActorDestroyed()
{
	RayActor = nullptr;
}

void UHvInterface::BeginPlay()
{
	EndPlay();

	UWorld *world = GetGameWorld();
	if (world)
	{
		RayActor = world->SpawnActor<AHVRayActor>(AHVRayActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		RayActor->SetRayHand(RayHandIndex);
	}
}

void UHvInterface::EndPlay()
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

AHVRayActor *UHvInterface::GetActor() {
	return RayActor;
}

void UHvInterface::onEventBuyResult(const char *sRet)
{
	TSharedPtr<FJsonValue> json_parsed;
	TSharedRef< TJsonReader<TCHAR> > json_reader = TJsonReaderFactory<TCHAR>::Create(UTF8_TO_TCHAR(sRet));
	if (FJsonSerializer::Deserialize(json_reader, json_parsed))
	{
		int ret = json_parsed->AsObject()->GetIntegerField("retCode");
		FString msg = json_parsed->AsObject()->GetStringField("retMsg");
		FString in_trade_no = json_parsed->AsObject()->GetStringField("intradeno");
		if (ret != 0)
		{
			UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"),
				FText::Format(HVSTRING("BuyFaild")
					, FFormatArgumentValue(FText::FromString(msg))), nullptr, EMsgBoxFlag::Ok);
		}
		else 
		{
			UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"),
				FText::Format(HVSTRING("BuySuccess")
					, FFormatArgumentValue(FText::FromString(in_trade_no))), nullptr, EMsgBoxFlag::Ok);

			if (BuyCallBack) {
				BuyCallBack(BuyParam.sItem, BuyParam.sOutTradeNo, in_trade_no, msg);
			}

			UWorld *world = GetGameWorld();
			if (world && UHvInterface::GetInstance()->GetActor()) {
				UHvInterface::GetInstance()->GetActor()->OnBuyResult.Broadcast(BuyParam.sItem, BuyParam.sOutTradeNo, in_trade_no, msg);
			}
		}
	}
}
void UHvInterface::BuyMsgBoxCallBack(int32 btnIndex)
{
	if (1 == btnIndex)
	{
		if (funcBuy) {
			funcBuy(TCHAR_TO_ANSI(*BuyParam.sItem), BuyParam.nPrice, TCHAR_TO_ANSI(*BuyParam.sOutTradeNo));
		}
	}
}

void UHvInterface::ResumeGameMsgBoxCallBack(int32 btnIndex)
{
	if (1 == btnIndex)
	{
		if (ResumeGameInfo.isAgain != 0) {
			sendCommand(3, "", 0);
		}
	}
}

void UHvInterface::onEventResumeGame(const char *sRet)
{
	if (GetInstance()->bDisableMenuBox) {
		return;
	}

	TSharedPtr<FJsonValue> json_parsed;
	TSharedRef< TJsonReader<TCHAR> > json_reader = TJsonReaderFactory<TCHAR>::Create(UTF8_TO_TCHAR(sRet));
	if (FJsonSerializer::Deserialize(json_reader, json_parsed))
	{
		ResumeGameInfo.retCode = json_parsed->AsObject()->GetIntegerField("retCode");
		ResumeGameInfo.retMsg = json_parsed->AsObject()->GetStringField("retMsg");
		ResumeGameInfo.isAgain = json_parsed->AsObject()->GetIntegerField("isAgain");
		ResumeGameInfo.reAmt = json_parsed->AsObject()->GetNumberField("reAmt");
		ResumeGameInfo.reTicket = json_parsed->AsObject()->GetIntegerField("reTicket");
		
		UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"),
			   FText::Format(HVSTRING("ResumeNotice")
				   , FFormatArgumentValue(ResumeGameInfo.reAmt), FFormatArgumentValue(ResumeGameInfo.reTicket)), &UHvInterface::ResumeGameMsgBoxCallBack);

		UWorld *world = GetGameWorld();
		if (world && UHvInterface::GetInstance()->GetActor()) {
			UHvInterface::GetInstance()->GetActor()->OnResumeGame.Broadcast(ResumeGameInfo.reAmt, ResumeGameInfo.reTicket);
		}
	}
}

void UHvInterface::onEventResumeGameResult(const char *sRet)
{
	if (strlen(sRet) > 0) {
		UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"), HVSTRING("ResumeFaild"), nullptr, EMsgBoxFlag::Ok);
	}
	else {
		UHvInterface::GetInstance()->MsgBox(HVSTRING("Notice"), HVSTRING("ResumeSuccess"), nullptr, EMsgBoxFlag::Ok);
	}
}

void UHvInterface::onEventSystemMenu(const char *sRet)
{
	if (GetInstance()->bDisableMenuBox) {
		return;
	}

	if (AHVMenuBox::GetInstance() == nullptr) 
	{
		UWorld *world = GetGameWorld();
		if (world)	{
			world->SpawnActor<AHVMenuBox>(AHVMenuBox::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		}		
	}
}

void UHvInterface::setRayHand(int i)
{
	UHvInterface *thiz = GetInstance();
	thiz->RayHandIndex = i;

	if (thiz->RayActor) {
		thiz->RayActor->SetRayHand(i);
	}
}