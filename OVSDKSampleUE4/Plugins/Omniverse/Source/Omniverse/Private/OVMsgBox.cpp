#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "OVMsgBox.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AOVMsgBox::AOVMsgBox(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	BtnYes = NULL;
	BtnCancel = NULL;
	Title = NULL;
	Content = NULL;

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Omniverse/OVSDK/MessageBox.MessageBox_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) {
		RootWidget = (UUserWidget*)CreateWidget<UUserWidget>(world, win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);			
	}
}


void AOVMsgBox::Init(FText title, FText content, OVSDKMsgBoxCallback func, EMsgBoxFlag::Flag flag)
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

		BtnYes = Cast<UButton>(InitWidget("btn_yes", OVSTRING("Yes"), flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Ok));
		if (BtnYes) {
			BtnYes->OnClicked.AddDynamic(this, &AOVMsgBox::BtnYesClicked);
		}

		BtnCancel = Cast<UButton>(InitWidget("btn_cancel", OVSTRING("Cancel"), flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Cancel));
		if (BtnCancel) {
			BtnCancel->OnClicked.AddDynamic(this, &AOVMsgBox::BtnCancelClicked);
		}
	}
}

void AOVMsgBox::BtnYesClicked()
{
	if (CallbackFunc) {
		CallbackFunc(1);
	}

	OnDlgEvent("click", "1");

	GetWorld()->DestroyActor(this);
}

void AOVMsgBox::BtnCancelClicked()
{
	if (CallbackFunc) {
		CallbackFunc(2);
	}

	OnDlgEvent("click", "2");

	GetWorld()->DestroyActor(this);
}
