// Fill out your copyright notice in the Description page of Project Settings.

#include "HVSDKSampleUE4.h"
#include "SampleUIActor.h"
#include "Button.h"
#include "HVInterface.h"
#include "HVStringRes.h"
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

		UButton *btn = InitButton("Button_Purchase", FText::FromString("Purchase"));
		if (btn) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonPurchase);
		}
		btn = InitButton("Button_MsgBox", FText::FromString("MsgBox"));
		if (btn) {
			btn->OnClicked.AddDynamic(this, &ASampleUIActor::OnClickButtonMsgBox);
		}
		
		WidgetComponent->SetDrawAtDesiredSize(true);
	}

	bAlwaysOnCamera = false;
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
	UHvInterface::buy("TestItem", 1, FString("SampleTradeID_") + FString::FromInt(time(nullptr)));
}

void ASampleUIActor::OnClickButtonMsgBox() {
	UHvInterface::MsgBox(HVSTRING("HelloWorld"), FText::FromString(UHVStringRes::GetString("Welcome") + UHvInterface::getUserName()), nullptr, EMsgBoxFlag::Ok);
}
