// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/UMG/Public/UMG.h"
#include "ipc_head.h"
#include "HVMsgBox.h"
#include "HVInterface.generated.h"

#define HVSDK_VERSION "0.3.0"

UENUM(BlueprintType)
namespace ERayVisibility
{
	enum Type {
		Auto,
		AlwaysShow,
		AlwaysHide,
	};
}


typedef void(__stdcall *callback_func)(const char *sName, const char *sRet, unsigned nRetLen, void *pUserData);

typedef void(*HvInit)(int nGameId, const char *sGameKey, const char *sParams, callback_func cb, void *pUserData);
typedef void(*HvDrive)();
typedef IPCDevice* (*HvGetDeviceInfo)();
typedef IPCUser* (*HvGetUserInfo)();
typedef float(*HvGetOmniYawOffset)();
typedef void (*HvBuy)(const char *sItem, double nPrice, const char *sOutTradeNo);
typedef void (*HvSendCommand)(unsigned nCmd, const char *sData, unsigned nLen);
typedef bool (*HvIsGuest)();
typedef const char* (*HvGetSysLang)();

extern HvInit funcInit;
extern HvDrive funcDrive;
extern HvGetDeviceInfo funcGetDeviceInfo;
extern HvGetUserInfo funcGetUserInfo;
extern HvGetOmniYawOffset funcGetOmniYawOffset;
extern HvBuy funcBuy;
extern HvSendCommand funcSendCommand;
extern HvIsGuest funcIsGuest;
extern HvGetSysLang;

struct FBuyParam
{
	FString sItem;
	double nPrice;
	FString sOutTradeNo;
};
typedef void(*HVBUY_CALLBACK)(FString sItem, FString sOutTradeNo, FString sInTradeNo, FString sErr);
struct FResumeGameInfo
{
	int retCode;
	FString retMsg;
	int isAgain;
	double reAmt;
	int reTicket;
};

class AHVMsgBox;
class AHVRayActor;
class AHVDlgBase;
/**
 * 
 */
UCLASS()
class OMNIVERSE_API UHvInterface : public UBlueprintFunctionLibrary, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()
	friend class AHVRayActor;
	friend class AHVDlgBase;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init", Keywords = "Init"), Category = "HVSDK")
	static void init(int nGameId, FString sGameKey, FString sParam);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "HasInit", Keywords = "HasInit"), Category = "HVSDK")
	static bool hasInit();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IsJustInitDone", Keywords = "IsJustInitDone"), Category = "HVSDK")
	static bool isJustInitDone();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetInitRet", Keywords = "GetInitRet"), Category = "HVSDK")
	static FString getInitRet();

	DEPRECATED(4.0, "drive has implemented by UHvInterface::Tick().")
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Drive", Keywords = "Drive"), Category = "HVSDK")
	static void drive();

	static IPCDevice* getDeviceInfo();
	static IPCUser* getUserInfo();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetDeviceNo", Keywords = "GetDeviceNo"), Category = "HVSDK")
	static FString getDeviceNo();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetDeviceId", Keywords = "GetDeviceId"), Category = "HVSDK")
	static int getDeviceId();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetShopId", Keywords = "GetShopId"), Category = "HVSDK")
	static int getShopId();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUserName", Keywords = "GetUserName"), Category = "HVSDK")
	static FString getUserName();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUserId", Keywords = "GetUserId"), Category = "HVSDK")
	static int getUserId();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUserPhone", Keywords = "GetUserPhone"), Category = "HVSDK")
	static FString getUserPhone();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUserEmail", Keywords = "GetUserEmail"), Category = "HVSDK")
	static FString getUserEmail();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUserWallet", Keywords = "GetUserWallet"), Category = "HVSDK")
	static float getUserWallet();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetGamePrepareLeft", Keywords = "GetGamePrepareLeft"), Category = "HVSDK")
	static float getGamePrepareLeft();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetGameDurationLeft", Keywords = "GetGameDurationLeft"), Category = "HVSDK")
	static float getGameDurationLeft();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetGamePrepare", Keywords = "GetGamePrepare"), Category = "HVSDK")
	static float getGamePrepare();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetGameDuration", Keywords = "GetGameDuration"), Category = "HVSDK")
	static float getGameDuration();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetBillingMode", Keywords = "GetBillingMode"), Category = "HVSDK")
	static FString getBillingMode();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OmniYawOffset", Keywords = "OmniYawOffset"), Category = "HVSDK")
	static float getOmniYawOffset();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Buy", Keywords = "Buy"), Category = "HVSDK")
	static void buy(FString sItem, float nPrice, FString sOutTradeNo);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SendCommand", Keywords = "SendCommand"), Category = "HVSDK")
	static void sendCommand(int nCmd, FString sData, int nLen);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IsGuest", Keywords = "IsGuest"), Category = "HVSDK")
	static bool isGuest();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ConfirmPlayGame", Keywords = "ConfirmPlayGame"), Category = "HVSDK")
	static void confirmPlayGame();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetRayHand", Keywords = "SetRayHand"), Category = "HVSDK")
	static void setRayHand(int Index);

	UPROPERTY(BlueprintReadWrite, Category = "HVSDK")
	float UmgDistance;

	UPROPERTY(BlueprintReadWrite, Category = "HVSDK")
	bool bDisableMenuBox;

	UPROPERTY(BlueprintReadWrite, Category = "HVSDK")
	bool bDisableResumeGame;

	UPROPERTY(BlueprintReadWrite, Category = "HVSDK")
	TEnumAsByte<ERayVisibility::Type> RayVisibility;
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UvInterfaceInstance", Keywords = "UvInterfaceInstance"), Category = "HVSDK")
	static UHvInterface* GetInstance();

	// FTickableGameObject interface
	virtual bool IsTickable() const override {
		return true;
	}
	virtual TStatId GetStatId() const override	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UHvInterface, STATGROUP_Tickables);
	}
	virtual void Tick(float DeltaTime) override;
	// End of FTickableGameObject interface

	static AHVDlgBase *MsgBox(FText Title, FText Content, HVMSGBOX_CALLBACK Func, EMsgBoxFlag::Flag Flag = EMsgBoxFlag::OkCancel);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "MsgBox", Keywords = "MsgBox"), Category = "HVSDK")
	static AHVDlgBase *MsgBoxBP(FText Title, FText Content, EMsgBoxFlag::Flag Flag = EMsgBoxFlag::OkCancel);

	UFUNCTION(BlueprintCallable, Category = "HVSDK")
	static AHVDlgBase *ShowDlgJson(const FString &jsonFilePrefix);

	UFUNCTION(BlueprintCallable, Category = "HVSDK")
	static AHVDlgBase *ShowDlgBP(TSubclassOf<UUserWidget> uiAsset, FVector loc = FVector::ZeroVector, FRotator rot = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "HVSDK")
	static AHVDlgBase *ShowDlgPrompt(FText text, FVector loc = FVector::ZeroVector, FRotator rot = FRotator::ZeroRotator, bool alwaysOnCamera = true, bool billboard = false, float duration = 2.0f, float fadeout = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "HVSDK")
	static void UpdateDlgJsonWidgetCmd(const FString &jsonFilePrefix, const FString &widgetName, const FString &cmd);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetActor", Keywords = "GetActor"), Category = "HVSDK")
	static AHVRayActor *GetActor();

	void RegisterBuyCallBack(HVBUY_CALLBACK BuyCallBack);

	static int GetDlgNum() {
		return DlgList.Num();
	}

	static UWorld* GetGameWorld();
private:
	static UHvInterface *Instance;

	static HVBUY_CALLBACK BuyCallBack;
	static FBuyParam BuyParam;
	static void BuyMsgBoxCallBack(int32 BtnIndex);

	static void ResumeGameMsgBoxCallBack(int32 BtnIndex);
	static FResumeGameInfo ResumeGameInfo;

	//UPROPERTY()
	static TArray<AHVDlgBase*> DlgList;
	static void BeginPlayDlgBase(AHVDlgBase *Dlg);
	static void EndPlayDlgBase(AHVDlgBase *Dlg);

	static AHVRayActor *RayActor;
	static int RayHandIndex;
	static void OnRayActorDestroyed();

	static void BeginPlay();
	static void EndPlay();

	static void __stdcall dllcallback(const char *sName, const char *sRet, unsigned nRetLen, void *pUserData);
	static void onEventBuyResult(const char *sRet);
	static void onEventResumeGame(const char *sRet);
	static void onEventResumeGameResult(const char *sRet);
	static void onEventSystemMenu(const char *sRet);
public:
	static void onEventUpdateDlgJsonCmd(const char *sRet);
};