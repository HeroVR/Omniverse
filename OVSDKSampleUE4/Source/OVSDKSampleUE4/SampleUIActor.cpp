// Fill out your copyright notice in the Description page of Project Settings.

#include "OVSDKSampleUE4.h"
#include "SampleUIActor.h"
#include "Button.h"
#include "OVInterface.h"
#include "OVStringRes.h"
#include <time.h>

// Sets default values
ASampleUIActor::ASampleUIActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Game/SampleUI.SampleUI_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) 
	{
		RootWidget = (UUserWidget*)CreateWidget<UUserWidget>(world, win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);

		UButton *btn = Cast<UButton>(InitWidget("Button_Purchase", FText::FromString("Purchase")));
		if (btn) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonPurchase);
		}

		if (btn = Cast<UButton>(InitWidget("Button_MsgBox", FText::FromString("MsgBox")))) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonMsgBox);
		}

		if (btn = Cast<UButton>(InitWidget("Button_Save", FText::FromString("SaveGame")))) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonSaveGame);
		}
		if (btn = Cast<UButton>(InitWidget("Button_Load", FText::FromString("LoadGame")))) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonLoadGame);
		}
		
		WidgetComponent->SetDrawAtDesiredSize(true);
	}

	bAlwaysOnCamera = false;

	DlgSaving = DlgLoading = nullptr;

	if (!this->HasAnyFlags(RF_ClassDefaultObject))
	{
		UOVInterface::GetInstance()->OnSaveGameData.AddDynamic(this, &ASampleUIActor::OnSaveGameDone);
		UOVInterface::GetInstance()->OnLoadGameData.AddDynamic(this, &ASampleUIActor::OnLoadGameDone);
		UOVInterface::GetInstance()->OnBuyResult.AddDynamic(this, &ASampleUIActor::OnPurchaseDone);
	}	
}

// Called when the game starts or when spawned
void ASampleUIActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASampleUIActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASampleUIActor::OnClickButtonPurchase() {
	UOVInterface::Buy("TestItem", 1, FString("SampleTradeID_") + FString::FromInt(time(nullptr)));
}

void ASampleUIActor::OnPurchaseDone(FString itemName, FString outTradeNo, FString inTradeNo, FString errMsg)
{

}

void ASampleUIActor::OnClickButtonMsgBox() 
{
	UOVInterface::MsgBox(OVSTRING("HelloWorld"), FText::FromString(UOVStringRes::GetString("Welcome") + UOVInterface::GetUserName()), nullptr, EMsgBoxFlag::Ok);
	//UOVInterface::onEventUpdateDlgJsonCmd("test HHHH 1|Game time left: {0} sec.");
}

static void LookupCube(UWorld *world, AActor *&redCube, AActor *&greenCube)
{
	for (TActorIterator<AActor> i(world, AStaticMeshActor::StaticClass()); i; ++i)
	{
		AActor* actor = *i;
		if (!actor->IsPendingKill())
		{
			if (nullptr == redCube && actor->GetName() == "CubeRed") {
				redCube = actor;
			}
			else if (nullptr == greenCube && actor->GetName() == "CubeGreen") {
				greenCube = actor;
			}
		}
	}
}

void ASampleUIActor::OnClickButtonSaveGame()
{
	if (DlgSaving != nullptr) {
		return;
	}

	AActor *redCube = nullptr, *greenCube = nullptr;
	LookupCube(GetWorld(), redCube, greenCube);

	USaveGameSample *data = NewObject<USaveGameSample>();
	if (redCube) {
		data->RedTransform = redCube->GetActorTransform();
	}
	if (greenCube) {
		data->GreenTransform = greenCube->GetActorTransform();
	}
	
	if (UOVInterface::SaveGameData(data))	{
		DlgSaving = UOVInterface::MsgBox(FText::FromString("Save game"), FText::FromString("Processing, please standby."), nullptr, EMsgBoxFlag::None);
	}
}

void ASampleUIActor::OnClickButtonLoadGame()
{	
	if (UOVInterface::LoadGameData()) {
		DlgLoading = UOVInterface::MsgBox(FText::FromString("Load game"), FText::FromString("Processing, please standby."), nullptr, EMsgBoxFlag::None);
	}
}

void ASampleUIActor::OnSaveGameDone(int retCode, FString retMsg)
{
	if (DlgSaving) {
		DlgSaving->Destroy();
		DlgSaving = nullptr;
	}

	if (retCode == 0) {
		UOVInterface::MsgBox(FText::FromString("Save game"), FText::FromString("Success!"), nullptr, EMsgBoxFlag::Ok);
	}
	else {
		UOVInterface::MsgBox(FText::FromString("Save game failed"), FText::FromString(retMsg), nullptr, EMsgBoxFlag::Ok);
	}
}

void ASampleUIActor::OnLoadGameDone(int retCode, FString retMsg, USaveGame *data)
{
	if (DlgLoading) {
		DlgLoading->Destroy();
		DlgLoading = nullptr;
	}

	USaveGameSample *mydata = Cast<USaveGameSample>(data);
	if (nullptr != mydata)
	{
		AActor *redCube = nullptr, *greenCube = nullptr;
		LookupCube(GetWorld(), redCube, greenCube);

		if (redCube) {
			redCube->SetActorTransform(mydata->RedTransform);
		}
		if (greenCube) {
			greenCube->SetActorTransform(mydata->GreenTransform);
		}
	}	
}
