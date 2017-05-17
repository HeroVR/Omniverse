#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "HVMenuBox.h"
#include "WidgetComponent.h"

#pragma warning(disable: 4996)

AHVDlgJson::AHVDlgJson(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Omniverse/HvSDK/MenuBox.MenuBox_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> btn_bp(TEXT("Class'/Omniverse/HvSDK/Button.Button_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> txt_bp(TEXT("Class'/Omniverse/HvSDK/Text.Text_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) {
		RootWidget = CreateWidget<UUserWidget>(GetWorld(), win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);
	}

	ButtonClass = btn_bp.Object;
	TextClass = txt_bp.Object;
}


AHVDlgJson::~AHVDlgJson()
{
}

bool AHVDlgJson::LoadJson(const char *jsonFilePrefix)
{
	JsonPrefix = UTF8_TO_TCHAR(jsonFilePrefix);

	UPanelWidget *panel = RootWidget ? (UPanelWidget*)RootWidget->GetRootWidget() : nullptr;
	if (nullptr == panel) {
		return false;
	}

	char path[256];
	strcpy(path, UHvInterface::getUserInfo()->sConsolePath);
	strcat(path, "\\");
	strcat(path, jsonFilePrefix);
	strcat(path, "_ue4.json");
	char *json = LoadFile(path);
	if (nullptr == json) {
		return false;
	}
	
	TSharedRef< TJsonReader<TCHAR> > reader = TJsonReaderFactory<char>::Create(json);
	delete[] json;

	TSharedPtr<FJsonObject> values;
	if (FJsonSerializer::Deserialize(reader, values))
	{
		uint32 x = 0, y = 0, w = 0, h = 0;
		values->TryGetNumberField("x", x);
		values->TryGetNumberField("y", y);
		values->TryGetNumberField("w", w);
		values->TryGetNumberField("h", h);

		const TArray< TSharedPtr<FJsonValue> > &items = values->GetArrayField("Items");
		for (int i = 0; i < items.Num(); ++i)
		{
			const TSharedPtr<FJsonObject> *tt = nullptr;
			if (!items[i]->TryGetObject(tt) || nullptr == tt) {
				continue;
			}

			const FJsonObject &jo = **tt;
			jo.TryGetNumberField("x", x);
			jo.TryGetNumberField("y", y);
			jo.TryGetNumberField("w", w);
			jo.TryGetNumberField("h", h);

			FString name, type, style, cmd;
			jo.TryGetStringField("name", name);
			jo.TryGetStringField("type", type);
			jo.TryGetStringField("style", style);
			jo.TryGetStringField("cmd", cmd);

			NewWidget(panel, name, type, type == "Button" ? ButtonClass : TextClass, x, y, w, h, style, cmd);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Json " + values[i]->AsObject()->);
		}
	}
	return true;
}

bool AHVDlgJson::NewWidget(UPanelWidget *panel, const FString &name, const FString &type, TSubclassOf<class UObject> clazz, int x, int y, int w, int h, const FString &style, const FString &cmd)
{
	if (type != "Button" && type != "Text") {
		return false;
	}

	UWorld *world = GetWorld();
	if (nullptr == world) {
		return false;
	}

	UUserWidget *widget = CreateWidget<UUserWidget>(world, clazz);
	if (widget)
	{
		panel->AddChild(widget);

		UCanvasPanelSlot *slot = Cast<UCanvasPanelSlot>(widget->Slot);
		if (slot)
		{
			FAnchorData layout = slot->LayoutData;
			layout.Offsets.Left = x;
			layout.Offsets.Top = y;
			layout.Offsets.Right = w;
			layout.Offsets.Bottom = h;
			slot->SetLayout(layout);
		}

		UButton *btn = nullptr;
		UTextBlock *txtWidget = nullptr;
		if (type == "Button") 
		{
			if (nullptr != (btn = Cast<UButton>(widget->GetRootWidget())))	{
				txtWidget = Cast<UTextBlock>(btn->GetChildAt(0));
			}
		}
		else {
			txtWidget = Cast<UTextBlock>(widget->GetRootWidget());
		}
		
		UJsonWidget *mw = NewObject<UJsonWidget>();
		mw->AddToRoot();
		mw->Dlg = this;
		mw->Widget = widget;
		mw->Name = name;
		mw->Cmd = cmd;
		mw->TxtWidget = txtWidget;
		mw->DynaText = (txtWidget && UpdateText(txtWidget, cmd));

		if (btn) {
			btn->OnClicked.AddDynamic(&*mw, &UJsonWidget::OnClick);
		}

		JsonWidgets.Add(mw);
	}

	return widget != nullptr;
}

bool AHVDlgJson::UpdateText(UTextBlock *txtWidget, const FString &cmd)
{
	bool need_update = false;

	TArray<FString> cmds;
	if (cmd.ParseIntoArray(cmds, TEXT("|"), false) > 1)
	{
		int type = cmds[0].IsEmpty() ? 0 : FCString::Atoi(*cmds[0]);
		if (type == 0) {
			txtWidget->SetText(FText::FromString(cmds[1]));
		}
		else
		{
			FString content = "";
			IPCUser *ui = UHvInterface::getUserInfo();
			IPCDevice *di = UHvInterface::getDeviceInfo();
			switch (type)
			{
			case 1:
				content = FString::FromInt(ui->nGameDurationLeft);
				need_update = true;
				break;
			case 2:
				content = FString::FromInt(ui->nGamePrepareLeft);
				need_update = true;
				break;
			case 3:
				content = FString::FromInt(ui->nId);
				break;
			case 4:
				content = UTF8_TO_TCHAR(ui->sName);
				break;
			case 5:
				content = FString::FromInt(di->nShopId);
				break;
			case 6:
				content = UTF8_TO_TCHAR(di->sShopName);
				break;
			}

			TArray<FStringFormatArg> args;
			args.Add(content);		
			txtWidget->SetText(FText::FromString(FString::Format(*cmds[1], args)));
		}
	}
	return need_update;
}

bool AHVDlgJson::UpdateCmd(const FString &name, const FString &cmd)
{
	for (int i = 0; i < JsonWidgets.Num(); ++i)
	{
		UJsonWidget *jw = JsonWidgets[i];
		if (jw->Name == name)
		{
			jw->Cmd = cmd;
			jw->DynaText = (jw->TxtWidget && UpdateText(jw->TxtWidget, jw->Cmd));
			return true;
		}
	}
	return false;
}

void AHVDlgJson::EndPlay(const EEndPlayReason::Type reason)
{
	for (int i = 0; i < JsonWidgets.Num(); ++i) {
		JsonWidgets[i]->RemoveFromRoot();
	}
	JsonWidgets.Empty();

	Super::EndPlay(reason);
}

void AHVDlgJson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < JsonWidgets.Num(); ++i) 
	{
		if (JsonWidgets[i]->DynaText && JsonWidgets[i]->TxtWidget) {
			UpdateText(JsonWidgets[i]->TxtWidget, JsonWidgets[i]->Cmd);
		}
	}
}

extern HvSendCommand funcSendCommand;
void UJsonWidget::OnClick()
{
	TArray<FString> cmds;
	if (Cmd.ParseIntoArray(cmds, TEXT("|"), false) > 2)
	{
		int cmd_id = cmds[2].IsEmpty() ? 0 : FCString::Atoi(*cmds[2]);
		if (cmd_id && funcSendCommand) {
			funcSendCommand(cmd_id, "", 0);
		}

		if (GWorld && !(cmds.Num() > 3 && !cmds[3].IsEmpty() && FCString::Atoi(*cmds[3]) != 0))	{
			GWorld->DestroyActor(Dlg);
		}
	}
}

char* AHVDlgJson::LoadFile(const char *path)
{
	char *content = nullptr;
	FILE *file = fopen(path, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		size_t len = ftell(file);

		content = new char[len + 1];
		if (content)
		{
			fseek(file, 0, SEEK_SET);
			len = fread(content, 1, len, file);
			content[len] = 0;
		}

		fclose(file);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("UJsonWidget::LoadFile(%s) failed. "), UTF8_TO_TCHAR(path));
	}

	return content;
}