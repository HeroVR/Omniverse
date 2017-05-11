#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "HVMsgBox.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AHVMsgBox::AHVMsgBox(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	BtnYes = NULL;
	BtnCancel = NULL;
	Title = NULL;
	Content = NULL;

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Omniverse/HvSDK/MessageBox.MessageBox_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) {
		RootWidget = (UUserWidget*)CreateWidget<UUserWidget>(world, win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);			
	}
}


void AHVMsgBox::Init(FText title, FText content, HVMSGBOX_CALLBACK func, EMsgBoxFlag::Flag flag)
{
	if (RootWidget)
	{
		Title = (UTextBlock *)(RootWidget->GetWidgetFromName("title"));
		if (Title) {
			Title->SetText(title);
		}

		Content = (UTextBlock *)(RootWidget->GetWidgetFromName("content"));
		if (Content) {
			Content->SetText(content);
		}

		CallbackFunc = func;

		BtnYes = InitButton ("btn_yes", HVSTRING("Yes"), flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Ok);
		if (BtnYes) {
			BtnYes->OnClicked.AddDynamic(this, &AHVMsgBox::BtnYesClicked);
		}

		BtnCancel = InitButton("btn_cancel", HVSTRING("Cancel"), flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Cancel);
		if (BtnCancel) {
			BtnCancel->OnClicked.AddDynamic(this, &AHVMsgBox::BtnCancelClicked);
		}
	}
}

void AHVMsgBox::BtnYesClicked()
{
	if (CallbackFunc) {
		CallbackFunc(1);
	}

	OnDlgEvent("click", "1");

	GetWorld()->DestroyActor(this);
}

void AHVMsgBox::BtnCancelClicked()
{
	if (CallbackFunc) {
		CallbackFunc(2);
	}

	OnDlgEvent("click", "2");

	GetWorld()->DestroyActor(this);
}
