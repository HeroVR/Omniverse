#include "OmniversePrivatePCH.h"
#include "OVDlgJson.h"
#include "EngineGlobals.h"
#include "WidgetComponent.h"

#pragma warning(disable: 4996)

AOVDlgJson::AOVDlgJson(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), PreUserOmniCoupleRate(0)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Omniverse/OVSDK/MenuBox.MenuBox_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> btn_bp(TEXT("Class'/Omniverse/OVSDK/Button.Button_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> txt_bp(TEXT("Class'/Omniverse/OVSDK/Text.Text_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> slider_bp(TEXT("Class'/Omniverse/OVSDK/Slider.Slider_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> toggle_bp(TEXT("Class'/Omniverse/OVSDK/Toggle.Toggle_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) {
		RootWidget = CreateWidget<UUserWidget>(GetWorld(), win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);
	}

	ButtonClass = btn_bp.Object;
	TextClass = txt_bp.Object;
	SliderClass = slider_bp.Object;
	ToggleClass = toggle_bp.Object;
}


AOVDlgJson::~AOVDlgJson()
{
}

bool AOVDlgJson::LoadJson(const char *jsonFilePrefix)
{
	JsonPrefix = UTF8_TO_TCHAR(jsonFilePrefix);

	UPanelWidget *panel = RootWidget ? (UPanelWidget*)RootWidget->GetRootWidget() : nullptr;
	if (nullptr == panel) {
		return false;
	}

	char path[256];
	strcpy(path, UOVInterface::GetUserInfo()->sConsolePath);
	strcat(path, "\\");
	strcat(path, jsonFilePrefix);
	strcat(path, "_ue4.json");
	char *json = LoadFile(path);
	if (nullptr == json) {
		return false;
	}
	
	TSharedRef< TJsonReader<TCHAR> > reader = TJsonReaderFactory<TCHAR>::Create(UTF8_TO_TCHAR(json));
	delete[] json;

	TSharedPtr<FJsonObject> values;
	if (FJsonSerializer::Deserialize(reader, values))
	{
		uint32 x = 0, y = 0, w = 0, h = 0, msgid = 0;
		values->TryGetStringField("name", DlgJsonName);
		values->TryGetNumberField("w", w);
		values->TryGetNumberField("h", h);
		if (w > 0 && h > 0) 
		{
			WidgetComponent->SetDrawAtDesiredSize(false);
			WidgetComponent->SetDrawSize(FVector2D(w, h));
		}

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
			jo.TryGetNumberField("msgid", msgid);

			FString name, type, style, cmd;
			jo.TryGetStringField("name", name);
			jo.TryGetStringField("type", type);
			jo.TryGetStringField("style", style);
			jo.TryGetStringField("cmd", cmd);

			NewWidget(panel, name, type, x, y, w, h, msgid, style, cmd, jo);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Json " + values[i]->AsObject()->);
		}
	}
	return true;
}

bool AOVDlgJson::NewWidget(UPanelWidget *panel, const FString &name, const FString &type, int x, int y, int w, int h, int MsgId, const FString &style, const FString &cmd, const FJsonObject &jo)
{
	TSubclassOf<class UObject> clazz = TextClass;
	if (type == "Button") {
		clazz = ButtonClass;
	}
	else if (type == "Text") {
		clazz = TextClass;
	}
	else if (type == "Slider") {
		clazz = SliderClass;
	}
	else if (type == "Toggle") {
		clazz = ToggleClass;
	}
	else {
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

		UJsonWidget *mw = NewObject<UJsonWidget>();
		mw->AddToRoot();
		mw->Dlg = this;
		mw->Widget = widget;
		mw->Name = name;
		mw->Cmd = cmd;
		mw->MsgId = MsgId;

		UButton *btn = nullptr;
		UTextBlock *txtWidget = nullptr;
		if (type == "Button") 
		{
			int direct_close;
			if (jo.TryGetNumberField("close", direct_close)) {
				mw->WidgetData.DataForButton.DirectClose = (0 != direct_close);
			}
			else {
				mw->WidgetData.DataForButton.DirectClose = false;
			}
			if (nullptr != (btn = Cast<UButton>(widget->GetRootWidget())))	{
				txtWidget = Cast<UTextBlock>(btn->GetChildAt(0));
			}

			if (btn) {
				btn->OnClicked.AddDynamic(&*mw, &UJsonWidget::OnClick);
			}
		}
		else if (type == "Slider") 
		{
			if (name == "OmniCoupleRate")
			{
				USlider *sld = Cast<USlider>(widget->GetRootWidget());
				if (sld)
				{
					PreUserOmniCoupleRate = UOVInterface::GetUserOmniCoupleRate();

					sld->SetValue(UOVInterface::GetOmniCoupleRate());
					sld->OnValueChanged.AddDynamic(&*mw, &UJsonWidget::OnSlide);
				}
			}
		}
		else if (type == "Toggle")
		{
			if (name == "OmniCoupleMode")
			{
				UCheckBox *toggle = Cast<UCheckBox>(widget->GetRootWidget());
				if (toggle)
				{
					PreUserOmniCoupleRate = UOVInterface::GetUserOmniCoupleRate();

					toggle->SetCheckedState(UOVInterface::GetOmniCoupleRate() < 0.5f ? ECheckBoxState::Unchecked : ECheckBoxState::Checked);
					toggle->OnCheckStateChanged.AddDynamic(&*mw, &UJsonWidget::OnToggle);
				}
			}
		}
		else {
			txtWidget = Cast<UTextBlock>(widget->GetRootWidget());
		}
		
	
		mw->TxtWidget = txtWidget;
		mw->DynaText = (txtWidget && UpdateText(txtWidget, cmd));
		JsonWidgets.Add(mw);
	}

	return widget != nullptr;
}

bool AOVDlgJson::UpdateText(UTextBlock *txtWidget, const FString &cmd)
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
			IPCUser *ui = UOVInterface::GetUserInfo();
			IPCDevice *di = UOVInterface::GetDeviceInfo();
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
				content = FString::FromInt(ui->nUserId);
				break;
			case 4:
				content = UTF8_TO_TCHAR(ui->sUserName);
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

UJsonWidget* AOVDlgJson::LookupWidget(FString name)
{
	for (int i = 0; i < JsonWidgets.Num(); ++i)
	{
		UJsonWidget *jw = JsonWidgets[i];
		if (jw->Name == name) {
			return jw;
		}
	}
	return nullptr;
}

bool AOVDlgJson::UpdateCmd(const FString &name, const FString &cmd)
{
	UJsonWidget *jw = LookupWidget(name);
	if (jw)
	{
		jw->Cmd = cmd;
		jw->DynaText = (jw->TxtWidget && UpdateText(jw->TxtWidget, jw->Cmd));
		return true;
	}	
	return false;
}

void AOVDlgJson::TryClose(const FString &name)
{
	if ((0 == name.Len()) || name == DlgJsonName) {
		GWorld->DestroyActor(this);
	}
}

void AOVDlgJson::EndPlay(const EEndPlayReason::Type reason)
{
	uint32 currUserOmniCoupleRate = UOVInterface::GetUserOmniCoupleRate();
	if (currUserOmniCoupleRate != PreUserOmniCoupleRate)
	{
		UOVInterface::SendCommand(14, "", 0);
	}
	UOVInterface::SendCommand(15, DlgJsonName, DlgJsonName.Len());
	for (int i = 0; i < JsonWidgets.Num(); ++i) {
		JsonWidgets[i]->RemoveFromRoot();
	}
	JsonWidgets.Empty();

	Super::EndPlay(reason);
}

void AOVDlgJson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < JsonWidgets.Num(); ++i) 
	{
		if (JsonWidgets[i]->DynaText && JsonWidgets[i]->TxtWidget) {
			UpdateText(JsonWidgets[i]->TxtWidget, JsonWidgets[i]->Cmd);
		}
	}
}

void UJsonWidget::OnClick()
{
	if (WidgetData.DataForButton.DirectClose) {
		GWorld->DestroyActor(Dlg);
	}
	else {
		FString msg = "dlg=" + Dlg->GetDlgJsonName() + ";wgt=" + Name;
		UOVInterface::SendCommand(MsgId, msg, msg.Len());
	}
}

void UJsonWidget::OnSlide(float val)
{
	UOVInterface::SetOmniCoupleRate(val);
}

void UJsonWidget::OnToggle(bool state)
{
	UOVInterface::SetOmniCoupleMode(state);
}

char* AOVDlgJson::LoadFile(const char *path)
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