# **Omniverse** #
The repository is Omniverse SDK for Unity and UnrealEngine4. This document is edited by VSCode. You can open and preview this document by VSCode if there is any display error.
**********************************************************************
**********************************************************************
# **Projects** #
## OVSDKSampleUE4 ##
It's the sample for UE4 C++;
## OVSDKSampleUE4BP ##
It's the sample for UE4 Blueprint; In git repository, we keep one copy of Omniverse plugin to avoid confict. So you can make link Ominverse under OVSDKSampleUE4BP\Plugins to OVSDKSampleUE4\Plugins\Omniverse, like these (and make sure you got administrator privilege)  
``
mklink /D Omniverse ..\..\OVSDKSampleUE4\Plugins\Omniverse
``
## OVSDKSampleUnity for Unity;
It's the sample for unity, you can export unity plugin package here.
**********************************************************************
**********************************************************************
# **Usage** #
## Use Omniverse SDK by UE4 C++ 
1. Copy OVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
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
5. Call UOVInterface::Init(...) on game start.
```C++
UOVInterface::Init(1019, "01ec17dac7140c0fbe936ee128310000", "omni=1");
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
    OmniControllerComponent->InitOmniDone.AddDynamic(this, &AOVSDKSampleUE4Character::OnInitOmniDone);
    //OmniControllerComponent->Camera = FirstPersonCameraComponent;
```
6. Bind MoveForward and MoveRight in your character::SetupPlayerInputComponent(...) and implement MoveForward and MoveRight. You may already have done these :)

7. Done. You can study OVSDKSampleUE4 if any problems.
**********************************************************************
## Use Omniverse by UE4 blueprint
1. Copy OVSDKSampleUE4/Plugins/Omniverse to your project's Plugins;
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
4. Call OVSDK.Init when game begin. (you can do it in GameInstance::Init)
5. Config yourproject Engine.Input, add Omni X-Axis as MoveRight input, add Omni Y-Axis as MoveForward input. 
6. Modify character blueprint, add OmniControllerComponent, config OmniControllerComponent's Camera property. Implement InputAxis MoveForward/MoveRight;
7. Done. You can study OVSDKSampleUE4BP if any problems.
**********************************************************************
## Use Omniverse by Unity
1. Import Omniverse.unitypackage. (You can export from OVSDKSampleUnity)
2. Config OmniControllerViveSetup, refer to OmniSDK documents;
3. Call OVSDK.Init when game start.
4. Done. You can study OVSDKSampleUnity if any problems.
**********************************************************************
**********************************************************************
# **Testing** #
- Press Ctrl + Alt + Shift + O in game, you should see an Omniverse pop-up message box;
- Create a text file under Plugins\Omniverse\DLL\Win64(same as Omniverse.Functions.dll path), named as devenv.cfg. The text file's content sample is listed below.
The text file simulates the retail omniverse enviroment, you can modify user id, prepare-time (unit: seconds), game-time. After UOVInterface::HasInitialized return true, you can call UOVInterface::GetUserInfo() to get player's information.
```
user=33
preparetime=15
billingmode=timing
gametime=360
```
 
    