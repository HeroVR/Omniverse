#Omniverse
Omniverse, Unity, UnrealEngine4

## HVSDKSampleUE4 ##
It's the sample for UE4 C++;
##HVSDKSampleUE4_BP ##
It's the sample for UE4 Blueprint; In git repository, we keep one copy of Omniverse plugin to avoid confict. So you can make link Ominverse under HVSDKSampleUE4_BP\Plugins to HVSDKSampleUE4\Plugins\Omniverse, like these (and make sure you got administrator privilege)  
``
mklink /D Omniverse ..\..\HVSDKSampleUE4\Plugins\Omniverse
``
##HVSDKSampleUnity for Unity;
It's the sample for unity, you can export unity plugin package here.
##Use Omniverse by UE4 C++ 
1. Copy HVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
2. Add plugins config to uproject:  
``
	"Plugins": [  
		{  
			"Name": "Omniverse",  
			"Enabled": true  
		},
``
3. Add dependency module in yourproject.build.cs:  
``
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Omniverse"});  
``
4. Config yourproject Engine.Input, add Omni X-Axis as MoveRight input, add Omni Y-Axis as MoveForward input. 
5. Call HVSDK::init() on game start.
``
HVSDK::init
``
5. Add UOmniControllerComponent to your ACharacter. Please config UOmniControllerComponent::Camera correctly. 
UOmniControllerComponent need HDM camera to correct Omni harness orientation, make HDM work correctly when ACharacter's yaw changed by omni-harness turing.
Three ways to config UOmniControllerComponent::Camera:  
>a. by default, UOmniControllerComponentwill use the first UCameraComponent from owner ACharacter;  
>b. assign UOmniControllerComponent::Camera manually;  
>c. config UOmniControllerComponent::CameraTag, UOmniControllerComponent will try find UCameraComponent from owner ACharacter whose tags contain CameraTag;  
``
	// Create OmniControllerComponent
	OmniControllerComponent = CreateDefaultSubobject<UOmniControllerComponent>(TEXT("OmniControllerComponent"));
	OmniControllerComponent->InitOmniDone.AddDynamic(this, &AHVSDKSampleUE4Character::OnInitOmniDone);
	//OmniControllerComponent->Camera = FirstPersonCameraComponent;
``	
6. Bind MoveForward and MoveRight in your character::SetupPlayerInputComponent(...) and implement MoveForward and MoveRight. You may already have done these :)

6. Study HVSDKSampleUE4.
## Use Omniverse by UE4 blueprint
1. Copy HVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
2. Add plugins config to uproject:  
``
	"Plugins": [  
		{  
			"Name": "Omniverse",  
			"Enabled": true  
		},
``
3. Add dependency module in yourproject.build.cs:  
``
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Omniverse"});
``
4. Call HVSDK.Init when game begin. (you can do it in GameInstance::Init)
5. Config yourproject Engine.Input, add Omni X-Axis as MoveRight input, add Omni Y-Axis as MoveForward input. 
6. Modify character blueprint, add OmniControllerComponent, config OmniControllerComponent's Camera property. Implement InputAxis MoveForward/MoveRight;
7. Study HVSDKSampleUE4_BP.
## Use Omniverse by Unity
1. Import Omniverse.unitypackage. (You can export from HVSDKSampleUnity)
2. Config OmniControllerViveSetup, refer to OmniSDK documents;
3. Call HVSDK.Init when game start.
4. Study HVSDKSampleUnity.
## Omniverse interface
###UHvInterface::init
Initilaize Omniverse by GameID, GameKey, and extra parameters. Each game has an unique GameID, and each CP has an unique GameKey, you can get it from Omniverse platform. The extra parameter is usually set as "omni=1".
###UHvInterface::getDeviceInfo
Return omni deivce info: Omni device No., shop id, shop name, etc.
###UHvInterface::getUserInfo
Return account info logined: unique account ID, nick name, email/phone number, billing info, game time etc;
###UHvInterface::hasInit
To check if UHvInterface::init has ended, because UHvInterface::init is an asynchronize task.
###UHvInterface::isJustInitDone
UHvInterface::init has just done, you can initialize the user's game-data by user-info from UHvInterface;
###UHvInterface::getUserName and other similar function.
Return the broken IPCUser/IPCDevice info for blueprint;
###UHvInterface::buy
In-game purchase call;
###UHvInterface::isGuest
If the user is a guest or an Omniverse account;
###UHvInterface::getOmniYawOffset
Return Omni yaw calibration result, refer OmniControllerComponent.cpp;
###UHvInterface::sendCommand
Internal function;
###UHvInterface::MsgBox
Popup a Message box always in front of the HDM, and a controller-ray will appear and can operate the Message box. The controller-ray use an UWidgetInteractionComponent, be sure not confict with you game.
###UHvInterface::RayVisibility
To controle controller-ray visibility. Auto: ray will shown when any AHVDlgBase exists; AlwaysShow: ray alreay shown; AlwaysHide: ray always hidden;
###UHVStringRes::LoadString/UHVStringRes::GetString
Omniverse localization solution, optional.
###Interface in unity
The interface is declared in HVSDK.cs. It's similar to UHvInterface, maybe some little name difference.