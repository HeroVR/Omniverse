#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "HVMenuBox.h"
#include "HVDlgBase.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"

AHVDlgBase::AHVDlgBase(const FObjectInitializer& ObjectInitializer)
	: AActor(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	bIsStatic = true;
	bShowRay = true;
	bAlwaysOnCamera = true;

	RootWidget = NULL;
	DlgIndex = 0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetRelativeRotation(FRotator(0.0f, -180, 0.0f));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->ComponentTags.Add(TEXT("HVSDK"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterial> material(TEXT("/Engine/EngineMaterials/Widget3DPassThrough.Widget3DPassThrough"));
	WidgetComponent->SetMaterial(0, material.Get());
	WidgetComponent->SetDrawAtDesiredSize(true);
	
	SetTickGroup(ETickingGroup::TG_LastDemotable);
	SetActorRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
}

void AHVDlgBase::BeginPlay()
{
	Super::BeginPlay();

	UHvInterface::BeginPlayDlgBase(this);
}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 15
void AHVDlgBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#else 
void AHVDlgBase::EndPlay();
{
	Super::EndPlay();
#endif
	UHvInterface::EndPlayDlgBase(this);
}

UWorld* AHVDlgBase::GetWorld() const
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
		UE_LOG(LogTemp, Warning, TEXT("AHVDlgBase::GetWorld failed."));
	}

	return world;
}

void AHVDlgBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAlwaysOnCamera && GWorld)
	{
		APlayerController* controller = GWorld->GetFirstPlayerController();
		if (controller)
		{
			FVector camera_loc;
			FRotator camera_rot;
			controller->GetPlayerViewPoint(camera_loc, camera_rot);

			FVector camera_dir = camera_rot.Vector();
			FVector loc = camera_loc + camera_dir * (UHvInterface::GetInstance()->UmgDistance - 1.0f * DlgIndex);

			SetActorLocationAndRotation(loc, camera_rot);			
		}
	}
}

UButton* AHVDlgBase::InitButton(const char *name, FText txt, bool visible)
{
	UButton *btn = nullptr;
	if (RootWidget) 
	{
		UWidget *widget = RootWidget->GetWidgetFromName(name);
		if (widget)
		{

			if (widget->IsA(UButton::StaticClass())) {
				btn = (UButton*)widget;
			}
			else if (widget->IsA(UUserWidget::StaticClass())) {
				btn = Cast<UButton>(((UUserWidget*)widget)->GetRootWidget());
			}

			if (btn)
			{
				UTextBlock *tb = Cast<UTextBlock>(btn->GetChildAt(0));
				if (tb) {
					tb->SetText(txt);
				}

				btn->SetVisibility(visible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			}
		}
	}

	return btn;
}