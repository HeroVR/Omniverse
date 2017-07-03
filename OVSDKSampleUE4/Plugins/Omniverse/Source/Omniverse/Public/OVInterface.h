// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/UMG/Public/UMG.h"
#include "ipc_head.h"
#include "OVMsgBox.h"
#include "OVInterface.generated.h"

#ifdef GetUserName
#undef GetUserName
#endif

#define OVSDK_VERSION "0.3.0"

UENUM(BlueprintType)
namespace ERayVisibility
{
	enum Type {
		Auto,
		AlwaysShow,
		AlwaysHide,
	};
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FResumeGame, double, fAmt, int, nTicket);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBuyResult, FString, sItem, FString, sOutTradeNo, FString, sInTradeNo, FString, sErr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSaveGameData, int, retCode, FString, retMsg);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLoadGameData, int, retCode, FString, retMsg, USaveGame*, data);


typedef void (*DllCallback)(const char *name, int retCode, const char *result, unsigned resultLen, void *userData);
typedef void (*DllInit)(int gameId, const char *gameKey, const char *params, DllCallback cb, void *userData);
typedef void (*DllDrive)();
typedef IPCDevice* (*DllGetDeviceInfo)();
typedef IPCUser* (*DllGetUserInfo)();
typedef float(*DllGetOmniYawOffset)();
typedef void (*DllBuy)(const char *itemName, double price, const char *outTradeNo);
typedef void (*DllSendCommand)(unsigned cmd, const char *data, unsigned len);
typedef bool (*DllIsGuest)();
typedef const char* (*DllGetSysLang)();
typedef void (*DllSaveGameData)(const void *data, unsigned len);
typedef void (*DllLoadGameData)();

extern DllInit funcInit;
extern DllDrive funcDrive;
extern DllGetDeviceInfo funcGetDeviceInfo;
extern DllGetUserInfo funcGetUserInfo;
extern DllGetOmniYawOffset funcGetOmniYawOffset;
extern DllBuy funcBuy;
extern DllSendCommand funcSendCommand;
extern DllIsGuest funcIsGuest;
extern DllGetSysLang funcGetSysLang;
extern DllSaveGameData funcSaveGameData;
extern DllLoadGameData funcLoadGameData;

class AOVMsgBox;
class AOVRayActor;
class AOVDlgBase;
/**
 * 
 */
UCLASS()
class OMNIVERSE_API UOVInterface : public UBlueprintFunctionLibrary, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()
	friend class AOVRayActor;
	friend class AOVDlgBase;

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void Init(int gameId, FString gameKey, FString param);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static bool HasInitialized();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static bool JustInitialized();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static int GetInitResultCode();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetInitResultMsg();

	static IPCDevice* GetDeviceInfo();
	static IPCUser* GetUserInfo();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetDeviceNo();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static int GetDeviceId();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static int GetShopId();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetUserName();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static int GetUserId();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetUserPhone();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetUserEmail();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetUserBalance();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetGamePrepareLeft();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetGameDurationLeft();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetGamePrepare();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetGameDuration();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static FString GetBillingMode();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static float GetOmniCalibrationOffset();

	UFUNCTION(BlueprintCallable, Category = OVSDK)
	static float GetOmniCoupleRate();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void Buy(FString itemName, float price, FString outTradeNo);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void SendCommand(int cmd, FString data, int len);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static bool IsGuest();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void ConfirmPlayGame();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static bool SaveGameData(USaveGame *data);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static bool LoadGameData();

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void SetRayHand(int index);

	UPROPERTY(BlueprintReadWrite, Category = "OVSDK")
	float UmgDistance;

	UPROPERTY(BlueprintReadWrite, Category = "OVSDK")
	bool bDisableMenuBox;

	UPROPERTY(BlueprintReadWrite, Category = "OVSDK")
	bool bDisableResumeGame;

	UPROPERTY(BlueprintReadWrite, Category = "OVSDK")
	TEnumAsByte<ERayVisibility::Type> RayVisibility;
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UvInterfaceInstance", Keywords = "UvInterfaceInstance"), Category = "OVSDK")
	static UOVInterface* GetInstance();

	// FTickableGameObject interface
	virtual bool IsTickable() const override {
		return true;
	}
	virtual TStatId GetStatId() const override	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UOVInterface, STATGROUP_Tickables);
	}
	virtual void Tick(float DeltaTime) override;
	// End of FTickableGameObject interface

	static AOVDlgBase *MsgBox(FText Title, FText Content, OVSDKMsgBoxCallback Func, EMsgBoxFlag::Flag Flag = EMsgBoxFlag::OkCancel);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "MsgBox", Keywords = "MsgBox"), Category = "OVSDK")
	static AOVDlgBase *MsgBoxBP(FText Title, FText Content, EMsgBoxFlag::Flag Flag = EMsgBoxFlag::OkCancel);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static AOVDlgBase *ShowDlgJson(const FString &jsonFilePrefix);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static AOVDlgBase *ShowDlgBP(TSubclassOf<UUserWidget> uiAsset, FVector loc = FVector::ZeroVector, FRotator rot = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static AOVDlgBase *ShowDlgPrompt(FText text, FVector loc = FVector::ZeroVector, FRotator rot = FRotator::ZeroRotator, bool alwaysOnCamera = true, bool billboard = false, float duration = 2.0f, float fadeout = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "OVSDK")
	static void UpdateDlgJsonWidgetCmd(const FString &DlgJsonName, const FString &widgetName, const FString &cmd);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetActor", Keywords = "GetActor"), Category = "OVSDK")
	static AOVRayActor *GetActor();

	static int GetDlgNum() {
		return DlgList.Num();
	}

	UPROPERTY(BlueprintAssignable, Category = "OVSDK")
	FResumeGame OnResumeGame;

	UPROPERTY(BlueprintAssignable, Category = "OVSDK")
	FBuyResult OnBuyResult;

	UPROPERTY(BlueprintAssignable, Category = "OVSDK")
	FSaveGameData OnSaveGameData;

	UPROPERTY(BlueprintAssignable, Category = "OVSDK")
	FLoadGameData OnLoadGameData;

	static UWorld* GetGameWorld();
private:
	static UOVInterface *Instance;

	static void BuyMsgBoxCallBack(int32 btnIndex);
	static void ResumeGameMsgBoxCallBack(int32 btnIndex);

	//UPROPERTY()
	static TArray<AOVDlgBase*> DlgList;
	static void BeginPlayDlgBase(AOVDlgBase *dlg);
	static void EndPlayDlgBase(AOVDlgBase *dlg);

	static AOVRayActor *RayActor;
	static int RayHandIndex;
	static void OnRayActorDestroyed();

	static void BeginPlay();
	static void EndPlay();

	static void __stdcall onEventFromDll(const char *name, int retCode, const char *result, unsigned resultLen, void *userData);
	static void onEventBuyResult(const char *result);
	static void onEventResumeGame(const char *result);
	static void onEventResumeGameResult(const char *result);
	static void onEventSystemMenu(const char *result);
	static void onEventSaveGame(int retCode, const char *result);
	static void onEventLoadGame(int retCode, const char *result, unsigned resultLen);
public:
	static void onEventUpdateDlgJsonCmd(const char *result);
	static void onEventCloseDlgJsonCmd(const char *result);
};