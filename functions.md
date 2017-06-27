# **Omniverse interface functions** #
The interface functions are declared in HVInterface.h. The game must invoke UOVInterface::Init. Other functions are optional. The functions not listed below are for internal usage. 
## *Initialize functions*
### void UOVInterface::Init(GameID, GameKey, ExParam)
>This function must be called in each game. It initialize Omniverse by GameID, GameKey, and extra parameters. Each game has an unique GameID, and each CP has an unique GameKey, 
you can get it from Omniverse platform. The extra parameter is usually set as "omni=1". The function returns immediately, but the initializing process  is an asynchronize task, so you must wait until the process done.
### bool UOVInterface::HasInitialized()
>Return true, if UOVInterface::Init process has done. 
### bool UOVInterface::JustInitialized()
>Return true, if UOVInterface::Init has just done in this game frame. You can initialize the user's game-data by user-info from UOVInterface.
Must after UOVInterface::Init done, you can call UOVInterface::getUserInfo, UOVInterface::getDeviceInfo serial functions.
### FString UOVInterface::GetInitResult()
>Return initialize result message.
### int UOVInterface::GetInitResultCode()
>Return initialize result code, 0 means successfully, otherelse means failed with error in result message.
## *Get Shop,Player information*
### IPCDevice* UOVInterface::GetDeviceInfo()
>Return omni deivce info: Omni device No., shop id, shop name, etc. struct IPCDevie is defined as
```C++
struct IPCDevice {
	char sNo[128];        //Omni serial-number;
	char sUID[64];        //Omni device UID in Omniverse backend.
	char sShopName[64];   //Shop name
	Uint32 nId;           //for internal usage
	Uint32 nShopId;	      //Shop Id
	char sCategory[64];   //for internal usage
};
```
### IPCUser* UOVInterface::GetUserInfo()
>Return account info logined: unique account ID, nick name, email/phone number, billing info, game time etc;
struct IPCUser is defined in ipc_head.h

```C++
struct IPCUser 
{
   Uint32 nId;                     //The player's unique id in Omniverse; 
   Uint32 nGameId;                 //GameID from UOVInterface::Init 
   char sCookies[256];             //for internal usage 
   char sGameSDKType[4];         //Omniverse SDK type, "unty": unity-SDK, "ue4x": UE4-SDK;
   Uint32 nGameSDKVersion;       //Omniverse SDK version, version string 0xAAAA.0xBB.0xCC, format as uint32 0xAAAABBCC;
   Uint32 nGamePrepareLeft;        //Game-prepare time left, use for single game ticket mode;
   Uint32 nGameDurationLeft;       //Game-play time left;
   Uint32 nGamePrepare;            //Game-prepare time, each game can config its own prepare time. The time is used to choose level, match game, but the duration is limited.
                                   //If level or match start, you should call OVSDK.ConfirmPlayGame() to tell SDK countdown game-time now.
   Uint32 nGameDuration;           //Gameplay time;
   char sBillingMode[32];          //Ticket mode, "timing", "timingreal", "direct_game", "shiyu_coin", "game_auth", "timescount";
   Uint32 nUserProp;               //for internal usage;
   char sConsolePath[128];         //for internal usage;
   char sReserved[68];             //for internal usage;
   char sQrcode[64];               //The omniverse trade number for this game ticket;
   char sWeb2d[128];               //for internal usage;
   Uint32 nCurGameId;              //for internal usage;
   char sReserved2[60];            //for internal usage; 
   char sName[128];                //Player's nick game in Omniverse;
   char sPhone[32];                //Player's phone number registered in Omniverse; (maybe masked, some character replaced by '*' for privacy)
   char sEmail[128];               //Player's email registered in Omniverse; (maybe masked)
   double nWallet;                 //Player's balance in Omniverse; (In-game purchase costs this balance)
   char sIcon[128];                //Player's portrait icon url;
   Uint32 nDesktopDurationLeft;    //for internal usage; 
};
```
IPCUser::sBillingMode meaning:
|mode|description|
|-|-|
|timing|experience end when game time counting down over.|
|timingreal|experience end, when game time counting down in real time (except prepare time, hobby time, omni tutorial time, idle time ….). |
|timescount|like timing, but specialized a game.|
|game_auth|experience can only end by operator.|
|direct_game|like timingreal, but specialized a game.|
|shiyu_coin|like timescount, but billed from UNIS billing system.|

### const char* UOVInterface::GetUserName() and other similar function.
>Return the broken IPCUser/IPCDevice info for blueprint.
### bool UOVInterface::IsGuest()
>Return true, if the player is a guest; 
## *In-game purchase*
### void UOVInterface::Buy(FString sItem, float nPrice, FString sOutTradeNo)
> In-game purchase call; Paramter sItem: item name; sOutTradeNo: customized trader number by CP. UOVInterface::OnBuyResult will be fired when purchase done.
## *Other utility functions*
### float UOVInterface::GetOmniCalibrationOffset()
> Return Omni yaw calibration result, refer OmniControllerComponent.cpp;
### float UOVInterface::GetOmniCoupleRate()
> Return Omni couple/decouple rate, 0: fully coupled to camera, 10000: fully decoupled (follows torso/ring angle).
### UOVInterface::MsgBox(...)
> Popup a Message box always in front of the HDM, and a controller-ray will appear and can operate the Message box. The controller-ray use an UWidgetInteractionComponent, be sure not confict with you game.
This function mainly internal used for Omniverse SDK.
### int UOVInterface::GetDlgNum()
> Return visible Omniverse message-box count. The game should pause if necessary when Omniverse message-box pop up.
### UOVInterface::RayVisibility
> To controle controller-ray visibility. Auto: ray will shown when any message box exists; AlwaysShow: ray alreay shown; AlwaysHide: ray always hidden;
### void UOVInterface::ConfirmPlayGame()
> Tell SDK game prepare stage ended, and begin to count down game-time; (not implemented in UE4 SDK yet)
###	bool UOVInterface::SaveGameData(USaveGame *data)
> Save game data to Omniverse cloud. Each player has a unique game data for each game. Saving game data is a asynchronize process, UOVInterface::OnSaveGameData will be fired when saving done;
### bool UOVInterface::LoadGameData()
> Load game data from Omniverse cloud. UOVInterface::OnLoadGameData will be fired when loading done.
**********************************************************************
**********************************************************************
# **Omniverse Unity interface** #
> The functions are declared in OVSDK.cs. It's similar to UOVInterface, maybe some little name difference.

 
    