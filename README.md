# **Omniverse** #
The repository is Omniverse SDK for Unity and UnrealEngine4. This document is edited by VSCode. You can open and preview this document by VSCode if there is any display error.
**********************************************************************
**********************************************************************
# **Projects** #
## HVSDKSampleUE4 ##
It's the sample for UE4 C++;
## HVSDKSampleUE4_BP ##
It's the sample for UE4 Blueprint; In git repository, we keep one copy of Omniverse plugin to avoid confict. So you can make link Ominverse under HVSDKSampleUE4_BP\Plugins to HVSDKSampleUE4\Plugins\Omniverse, like these (and make sure you got administrator privilege)  
``
mklink /D Omniverse ..\..\HVSDKSampleUE4\Plugins\Omniverse
``
## HVSDKSampleUnity for Unity;
It's the sample for unity, you can export unity plugin package here.
**********************************************************************
**********************************************************************
# **Usage** #
## Use Omniverse SDK by UE4 C++ 
1. Copy HVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
2. Add plugins config to uproject:  
```JSON
   "Plugins": [  
      {  
         "Name": "Omniverse",  
         "Enabled": true  
      },
```
3. Add dependency module in yourproject.build.cs:  
```CS
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Omniverse"});  
```
4. Config yourproject Engine.Input, add Omni X-Axis as MoveRight input, add Omni Y-Axis as MoveForward input. 
5. Call UHvInterface::init(...) on game start.
```C++
UHvInterface::init(1019, "01ec17dac7140c0fbe936ee128310000", "omni=1");
```
5. Add UOmniControllerComponent to your ACharacter. Please config UOmniControllerComponent::Camera correctly. 
UOmniControllerComponent need HDM camera to correct Omni harness orientation, make HDM work correctly when ACharacter's yaw changed by omni-harness turing.
Three ways to config UOmniControllerComponent::Camera:  
    >a. by default, UOmniControllerComponentwill use the first UCameraComponent from owner ACharacter;  
    >b. assign UOmniControllerComponent::Camera manually;  
    >c. config UOmniControllerComponent::CameraTag, UOmniControllerComponent will try find UCameraComponent from owner ACharacter whose tags contain CameraTag;  
```CPP 
    // Create OmniControllerComponent
    OmniControllerComponent = CreateDefaultSubobject<UOmniControllerComponent>(TEXT("OmniControllerComponent"));
    OmniControllerComponent->InitOmniDone.AddDynamic(this, &AHVSDKSampleUE4Character::OnInitOmniDone);
    //OmniControllerComponent->Camera = FirstPersonCameraComponent;
```
6. Bind MoveForward and MoveRight in your character::SetupPlayerInputComponent(...) and implement MoveForward and MoveRight. You may already have done these :)

7. Done. You can study HVSDKSampleUE4 if any problems.
**********************************************************************
## Use Omniverse by UE4 blueprint
1. Copy HVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
2. Add plugins config to uproject:  
```JSON
   "Plugins": [  
      {  
         "Name": "Omniverse",  
         "Enabled": true  
      },
```
3. Add dependency module in yourproject.build.cs:  
```CS
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Omniverse"});
```
4. Call HVSDK.Init when game begin. (you can do it in GameInstance::Init)
5. Config yourproject Engine.Input, add Omni X-Axis as MoveRight input, add Omni Y-Axis as MoveForward input. 
6. Modify character blueprint, add OmniControllerComponent, config OmniControllerComponent's Camera property. Implement InputAxis MoveForward/MoveRight;
7. Done. You can study HVSDKSampleUE4_BP if any problems.
**********************************************************************
## Use Omniverse by Unity
1. Import Omniverse.unitypackage. (You can export from HVSDKSampleUnity)
2. Config OmniControllerViveSetup, refer to OmniSDK documents;
3. Call HVSDK.Init when game start.
4. Done. You can study HVSDKSampleUnity if any problems.
**********************************************************************
**********************************************************************
# **Omniverse UE4 interface** #
The interface functions are declared in HVInterface.h. The game must invoke UHvInterface::init. Other functions are optional. The functions not listed below are for internal usage. 
## *Initialize functions*
### void UHvInterface::init(GameID, GameKey, ExParam)
>This function must be called in each game. It initialize Omniverse by GameID, GameKey, and extra parameters. Each game has an unique GameID, and each CP has an unique GameKey, 
you can get it from Omniverse platform. The extra parameter is usually set as "omni=1". The function returns immediately, but the initializing process  is an asynchronize task, so you must wait until the process done.
### bool UHvInterface::hasInit()
>Return true, if UHvInterface::init process has done. 
### bool UHvInterface::isJustInitDone()
>Return true, if UHvInterface::init has just done in this game frame. You can initialize the user's game-data by user-info from UHvInterface.
Must after UHvInterface::init done, you can call UHvInterface::getUserInfo, UHvInterface::getDeviceInfo serial functions.
## *Get Shop,Player information*
### IPCDevice* UHvInterface::getDeviceInfo()
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
### IPCUser* UHvInterface::getUserInfo()
>Return account info logined: unique account ID, nick name, email/phone number, billing info, game time etc;
struct IPCUser is defined in ipc_head.h

```C++
struct IPCUser 
{
   Uint32 nId;                     //The player's unique id in Omniverse; 
   Uint32 nGameId;                 //GameID from UHvInterface::init 
   char sCookies[256];             //for internal usage 
   char sGameHVSDKType[4];         //Omniverse SDK type, "unty": unity-SDK, "ue4x": UE4-SDK;
   Uint32 nGameHVSDKVersion;       //Omniverse SDK version, version string 0xAAAA.0xBB.0xCC, format as uint32 0xAAAABBCC;
   Uint32 nGamePrepareLeft;        //Game-prepare time left, use for single game ticket mode;
   Uint32 nGameDurationLeft;       //Game-play time left;
   Uint32 nGamePrepare;            //Game-prepare time, each game can config its own prepare time. The time is used to choose level, match game, but the duration is limited.
                                   //If level or match start, you should call HVSDK.ConfirmPlayGame() to tell SDK countdown game-time now.
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
### const char* UHvInterface::getUserName() and other similar function.
>Return the broken IPCUser/IPCDevice info for blueprint.
### bool UHvInterface::isGuest()
>Return true, if the player is a guest; 
## *In-game purchase*
### void UHvInterface::buy(FString sItem, float nPrice, FString sOutTradeNo)
> In-game purchase call; Paramter sItem: item name; sOutTradeNo: customized trader number by CP;
### void UHvInterface::RegisterBuyCallBack(HVBUY_CALLBACK BuyCallBack)
> Register a callback function, invoked when purchase process done.
## *Other utility functions*
### float UHvInterface::getOmniYawOffset()
> Return Omni yaw calibration result, refer OmniControllerComponent.cpp;
### UHvInterface::MsgBox(...)
> Popup a Message box always in front of the HDM, and a controller-ray will appear and can operate the Message box. The controller-ray use an UWidgetInteractionComponent, be sure not confict with you game.
This function mainly internal used for Omniverse SDK.
### UHvInterface::GetDlgNum()
> Return visible Omniverse message-box count. The game should pause if necessary when Omniverse message-box pop up.
### UHvInterface::RayVisibility
> To controle controller-ray visibility. Auto: ray will shown when any message box exists; AlwaysShow: ray alreay shown; AlwaysHide: ray always hidden;
### UHvInterface::ConfirmPlayGame()
> Tell SDK game prepare stage ended, and begin to count down game-time; (not implemented in UE4 SDK yet)
**********************************************************************
**********************************************************************
# **Omniverse Unity interface** #
> The functions are declared in HVSDK.cs. It's similar to UHvInterface, maybe some little name difference. The name may modify to the same as UE4 later.<p>
HVSDK.cs uses raw pointer, so your project must enable -unsafe option. You can google how to enable -unsafe in Unity.
**********************************************************************
**********************************************************************
# **Testing** #
- Press Ctrl + Alt + Shift + O in game, you should see an Omniverse pop-up message box;
- Create a text file under Plugins\Omniverse\DLL\Win64(same as desktop_plugin.dll path), named as hvdevenv.cfg. The text file's content sample is listed below.
The text file simulates the retail omniverse enviroment, you can modify user id, prepare-time (unit: seconds), game-time. After UHvInterface::hasInit return true, you can call UHvInterface::getUserInfo() to get player's information.
```
user=33
preparetime=15
billingmode=timing
gametime=360
```
**********************************************************************
**********************************************************************
# **Backend interface** #
These interfaces are used for backend, MMO game server likely.
## http://web2d.vr.yingxiong.com/gateway/check_user
Usually game client commits player's info to game server. The server should verify player's infomation, make sure the player is valid, not faked.
The sample call from game-server is below:
``` 
http://web2d.vr.yingxiong.com/gateway/check_user?gameid=1&qrcode=190f00f16809e018dbafc49200b3d3da&sign=d299dd347f034a49779ecc5212e971cc
```
|Paramter name|Type and length|Required|Description|
|-|-|-|-|
|gameid|string(30)|yes|GameID as in UHvInterface::init|
|sign|string(64)|yes|Signature string for vertification of this call|
|signtype|string(10)|no|Should be MD5|
|qrcode|string(50)|yes|Omniverse internal trade number|

The return result is in json format:
|Result parameter|Type|Required|Description|
|-|-|-|-|
|retCode|string|yes|"0" meaning sucess|
|retMsg|string|yes|return message or errors|
|accountId|string|yes|Player's ID on Omniverse|
|nickName|string|yes|Player's nick name on Omniverse|
|tel|string|yes|Player's phone number|
|sign|string|yes|signature string| 
**********************************************************************
## In-game purechase callback
CP can configure in-game purchase callback URL on CP backend. When in-game purchase success, Omniverse backend will invoke this URL to confirm purchase result.
It's more secure than the client's notify message. If the game have a game server or backend, CP should implement purchase result on game server. Game server should 
verify the callback and send the virtual-item player purchased.

The parameters are sended as POST method, and parameters are listed as below:
|Parameter name|Type(Length)|Required|Description|
|-|-|-|-|-|
|retCode|string(40)|yes|purchase result code, '0' means success, otherelse is error code|
|retMsg|string(40)|yes|result message or error description|
|gameid|string(40)|yes|GameID (same as UHvInterface::init(...) parameter) |
|sign|string(64)|yes|signature|
|signtype|string(40)|no|default is MD5|
|price|string(10)|yes|purchase price, unit: same as IPCUser::nWallet|
|outtradeno|string(40)|yes|CP customized trade number, CP can combine virtual-item type in this parameter|
|intradeno|string(40)|no|Omniverse trade number. No this parameter if retCode != '0'.|
|paytime|string(20)|no|Purchase complete time, format is 'YYYY-MM-DD hh:mm:ss'. No this parameter if retCode != '0'.|
|paystatus|string(20)|no|Purchase process status(3 status: 'paying', 'success', 'failed'). No this parameter if retCode != '0'.|

CP should return 'success' if purchase complete, otherwise purchase failed. Ominiverse will try Max 7 times to invoke this URL after timeout.
**********************************************************************
## How to create signature
1. Sort all parameters except sign and signtype by character;
2. Catch up all sorted parameters string as GET-parameters;
3. Make MD5 of GET-parameter and GameKey;
4. Use the MD5 result as the signature.

The signature verification method is same with creation.  
For example:
1. Source parameters as below. (If verify signature, exclude sign and signtype parameter) 
```PHP
array(‘gameid=>’1,
      ’title’=>’apple’,
      ’price’=>’32.05’,
      ’outtradeno’=>’31634001365’
)
```
2. Sort parameters, and make a GET-parameter 
```
gameid=1&outtradeno=31634001365&price=32.05&title=apple
```
3. Make MD5 (GameKey = '8E6U3W6mF' for example)
```PHP
var sign = MD5(gameid=79b7ed12621&outtradeno=31634001365&price=32.05&title=applev8E6U3W6mF);
// sign should be '44115ea103b7b301b853b534e7818de6'
```
4. Use MD5 result as signature. A http GET url should like below. To verify signature, just compare the MD5 result to sign. 
```
http://url?gameid=1&outtradeno=31634001365&price=32.05&title=apple&sign=44115ea103b7b301b853b534e7818de6
```



 
    