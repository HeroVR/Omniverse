#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "OVDlgBase.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AOVDlgBase::AOVDlgBase(const FObjectInitializer& ObjectInitializer)
	: AActor(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	bShowRay = true;
	bAlwaysOnCamera = true;

	AutoCloseTime = 0;
	AutoFadeoutTime = 0.5f;
	AutoCloseTimeLeft = 0;

	RootWidget = NULL;
	DlgIndex = 0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetRelativeRotation(FRotator(0.0f, -180, 0.0f));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->ComponentTags.Add(TEXT("OVSDK"));
	static ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("/Engine/EngineMaterials/Widget3DPassThrough.Widget3DPassThrough"));
	WidgetComponent->SetMaterial(0, material.Object);
	WidgetComponent->SetDrawAtDesiredSize(true);
	
	SetTickGroup(ETickingGroup::TG_LastDemotable);
	SetActorRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	static ConstructorHelpers::FObjectFinder<UClass> dummy_prompt_for_package_bp(TEXT("Class'/Omniverse/OVSDK/Prompt.Prompt_C'"));
	if (dummy_prompt_for_package_bp.Object) {
	}
}

void AOVDlgBase::BeginPlay()
{
	Super::BeginPlay();

	AutoCloseTimeLeft = AutoCloseTime;

	//if (RootWidget)
	//{
	//	RootWidget->WidgetTree->ForEachWidget([&] (UWidget *widget) {
	//			UButton *btn = Cast<UButton>(widget);
	//			if (btn) {
	//				btn->OnClicked.AddDynamic(this, &AOVDlgBase::BtnClicked);
	//			}
	//		}
	//	);
	//}

	if (bShowRay) {
		UOVInterface::BeginPlayDlgBase(this);
	}
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 15
void AOVDlgBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#else 
void AOVDlgBase::EndPlay();
{
	Super::EndPlay();
#endif

	UOVInterface::EndPlayDlgBase(this);

	OnDlgEvent("end", "");
}

UWorld* AOVDlgBase::GetWorld() const
{
	UWorld *world = Super::GetWorld();
	if (world) {
		return world;
	}

	UObject* outer = GetOuter();
	for (; outer != nullptr; outer = outer->GetOuter())
	{
		world = Cast<UWorld>(outer);
		if (world) {
			break;
		}
	}

	if (nullptr == world && !HasAnyFlags(RF_ClassDefaultObject)) {
		UE_LOG(LogTemp, Warning, TEXT("AOVDlgBase::GetWorld failed."));
	}

	return world;
}

void AOVDlgBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GWorld)
	{
		if (AutoCloseTime > 0.001f)
		{
			AutoCloseTimeLeft -= DeltaTime;
			if (AutoCloseTimeLeft < 0.001f) {
				GWorld->DestroyActor(this);
				return;
			}

			if (AutoFadeoutTime > 0.001f && AutoCloseTimeLeft < AutoFadeoutTime) 
			{
				float alpha = AutoCloseTimeLeft / AutoFadeoutTime;
				WidgetComponent->GetMaterialInstance()->SetVectorParameterValue("TintColorAndOpacity", FLinearColor(1, 1, 1, alpha));
			}
		}

		if (bAlwaysOnCamera || bBillboard)
		{
			APlayerController* controller = GWorld->GetFirstPlayerController();
			if (controller)
			{
				FVector camera_loc;
				FRotator camera_rot;
				controller->GetPlayerViewPoint(camera_loc, camera_rot);

				if (bAlwaysOnCamera)
				{
					FVector camera_dir = camera_rot.Vector();
					FVector loc = camera_loc + camera_dir * (UOVInterface::GetInstance()->UmgDistance - 1.0f * DlgIndex);
					SetActorLocationAndRotation(loc, camera_rot);
				}			
				else {
					SetActorRotation(camera_rot);
				}
			}
		}
	}
}

UWidget* AOVDlgBase::InitWidget(FName name, FText txt, bool visible)
{
	UWidget *widget = nullptr;
	UButton *btn = nullptr;
	UTextBlock *tb = nullptr;
	if (RootWidget) 
	{
		widget = RootWidget->GetWidgetFromName(name);
		if (widget)
		{
			if (widget->IsA(UButton::StaticClass())) {
				btn = (UButton*)widget;
			}
			else if (widget->IsA(UTextBlock::StaticClass())) {
				tb = (UTextBlock *)widget;
			}
			else if (widget->IsA(UUserWidget::StaticClass())) 
			{
				btn = Cast<UButton>(((UUserWidget*)widget)->GetRootWidget());
				if (nullptr == btn) {
					tb = Cast<UTextBlock>(((UUserWidget*)widget)->GetRootWidget());
				}
			}

			if (btn) {
				tb = Cast<UTextBlock>(btn->GetChildAt(0));	
			}

			if (tb) {
				tb->SetText(txt);
			}

			widget->SetVisibility(visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
	}

	UWidget *ret = btn != nullptr ? btn : (tb != nullptr ? tb : widget);
	return ret;
}

void AOVDlgBase::SetAttribute(bool showRay, bool alwaysOnCamera, bool billboard, float duartion, float fadeout)
{
	if (bShowRay != showRay)
	{
		if (bShowRay) {
			UOVInterface::EndPlayDlgBase(this);
		}
		else {
			UOVInterface::BeginPlayDlgBase(this);
		}

		bShowRay = showRay;
	}

	bAlwaysOnCamera = alwaysOnCamera;
	bBillboard = billboard;

	AutoCloseTimeLeft = AutoCloseTime = duartion;
	AutoFadeoutTime = fadeout;
}

void AOVDlgBase::SetWidgetClass(TSubclassOf<UUserWidget> uiAsset)
{
	RootWidget = CreateWidget<UUserWidget>(GetWorld(), uiAsset);
	WidgetComponent->SetWidget(RootWidget);
}

void AOVDlgBase::OnDlgEvent_Implementation(const FString &type, const FString &param)
{
	DlgEvent.Broadcast(type, param);
}
