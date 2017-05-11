#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "HVRayActor.h"
#include "HVInterface.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include <Runtime/HeadMountedDisplay/Public/MotionControllerComponent.h>
#include <Runtime/UMG/Public/Components/WidgetInteractionComponent.h>
#include "CustomMeshComponent.h"

AHVRayActor::AHVRayActor()
{
#if UE_VERSION_GE(4, 13)
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultComponent"));

	PrimaryActorTick.bCanEverTick = true;
	bVisible = false;

	static ConstructorHelpers::FObjectFinderOptional<UMaterial> material(TEXT("/Omniverse/HvSDK/LineMaterial.LineMaterial"));
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//left controller
		MotionController_Left = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Left"));
		MotionController_Left->SetupAttachment(GetRootComponent());
		MotionController_Left->Hand = EControllerHand::Left;

		//right controller
		MotionController_Right = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Right"));
		MotionController_Right->SetupAttachment(GetRootComponent());
		MotionController_Right->Hand = EControllerHand::Right;

		//interaction
		WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
		WidgetInteraction->InteractionDistance = 2000;
		WidgetInteraction->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));

		WidgetInteraction->bShowDebug = true;
		WidgetInteraction->DebugColor = FLinearColor::Green;
		WidgetInteraction->InteractionSource = !HasAnyFlags(RF_ClassDefaultObject) && UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() ? EWidgetInteractionSource::Custom : EWidgetInteractionSource::Mouse;
		WidgetInteraction->bEnableHitTesting = bVisible;
		WidgetInteraction->SetupAttachment(UHvInterface::RayHandIndex ? MotionController_Right : MotionController_Left);

		//left ray
		CustomMesh = CreateDefaultSubobject<UCustomMeshComponent>(TEXT("CustemMesh"));
		CustomMesh->SetMaterial(0, material.Get());
		CustomMesh->SetupAttachment(WidgetInteraction);
		CustomMesh->SetTranslucentSortPriority(255);
	}

#endif
	//AutoReceiveInput = EAutoReceiveInput::Player0;
}

AHVRayActor::~AHVRayActor()
{
	UHvInterface::OnRayActorDestroyed();
}

#define LINE_WEIGHT 1.5f

void AHVRayActor::DrawLine(UCustomMeshComponent *CustoemMesh, float Distance, bool ShowHit)
{
	Distance -= 10;

	TArray<FVector> Vertexs;
	Vertexs.Add(FVector(0.0f, -LINE_WEIGHT, LINE_WEIGHT));
	Vertexs.Add(FVector(0.0f, LINE_WEIGHT, LINE_WEIGHT));
	Vertexs.Add(FVector(0.0f, LINE_WEIGHT, -LINE_WEIGHT));
	Vertexs.Add(FVector(0.0f, -LINE_WEIGHT, -LINE_WEIGHT));
	Vertexs.Add(FVector(Distance, 0.0f, 0.0f));

	Vertexs.Add(FVector(Distance, -LINE_WEIGHT, LINE_WEIGHT));
	Vertexs.Add(FVector(Distance, LINE_WEIGHT, LINE_WEIGHT));
	Vertexs.Add(FVector(Distance, LINE_WEIGHT, -LINE_WEIGHT));
	Vertexs.Add(FVector(Distance, -LINE_WEIGHT, -LINE_WEIGHT));

	TArray<FCustomMeshTriangle> Triangles;
	FCustomMeshTriangle Triangle;
	Triangle.Vertex0 = Vertexs[0];
	Triangle.Vertex1 = Vertexs[2];
	Triangle.Vertex2 = Vertexs[1];
	Triangles.Add(Triangle);

	Triangle.Vertex0 = Vertexs[0];
	Triangle.Vertex1 = Vertexs[3];
	Triangle.Vertex2 = Vertexs[2];
	Triangles.Add(Triangle);

	Triangle.Vertex0 = Vertexs[0];
	Triangle.Vertex1 = Vertexs[1];
	Triangle.Vertex2 = Vertexs[4];
	Triangles.Add(Triangle);

	Triangle.Vertex0 = Vertexs[1];
	Triangle.Vertex1 = Vertexs[2];
	Triangle.Vertex2 = Vertexs[4];
	Triangles.Add(Triangle);

	Triangle.Vertex0 = Vertexs[2];
	Triangle.Vertex1 = Vertexs[3];
	Triangle.Vertex2 = Vertexs[4];
	Triangles.Add(Triangle);

	Triangle.Vertex0 = Vertexs[3];
	Triangle.Vertex1 = Vertexs[0];
	Triangle.Vertex2 = Vertexs[4];
	Triangles.Add(Triangle);

	if (ShowHit)
	{
		Triangle.Vertex0 = Vertexs[5];
		Triangle.Vertex1 = Vertexs[7];
		Triangle.Vertex2 = Vertexs[6];
		Triangles.Add(Triangle);

		Triangle.Vertex0 = Vertexs[5];
		Triangle.Vertex1 = Vertexs[8];
		Triangle.Vertex2 = Vertexs[7];
		Triangles.Add(Triangle);

		Triangle.Vertex0 = Vertexs[5];
		Triangle.Vertex1 = Vertexs[6];
		Triangle.Vertex2 = Vertexs[7];
		Triangles.Add(Triangle);

		Triangle.Vertex0 = Vertexs[5];
		Triangle.Vertex1 = Vertexs[7];
		Triangle.Vertex2 = Vertexs[8];
		Triangles.Add(Triangle);
	}

	CustoemMesh->SetCustomMeshTriangles(Triangles);
}

void AHVRayActor::Tick(float DeltaSeconds)
{
	CheckVisible();
	if (!bVisible) {
		return;
	}

	bool hmd_enabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
	if (hmd_enabled)
	{
		APlayerController* pc = GWorld->GetFirstPlayerController();
		if (pc)
		{
			APlayerCameraManager *pcm = pc->PlayerCameraManager;
			if (pcm)
			{
				// keep AHVRayActor's coordination same with HDM;
				FVector hdm_loc, hdm_loc_world = pcm->GetCameraLocation();
				FRotator hdm_rot, hdm_rot_world = pcm->GetCameraRotation();
				UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hdm_rot, hdm_loc);

				FTransform hdm(hdm_rot, hdm_loc), hdm_world(hdm_rot_world, hdm_loc_world);
				hdm = hdm.Inverse();
				FTransform hdm_parent;
				FTransform::Multiply(&hdm_parent, &hdm, &hdm_world);
				SetActorTransform(hdm_parent);
			}
		}		
	}

	WidgetInteraction->InteractionSource = hmd_enabled ? EWidgetInteractionSource::Custom : EWidgetInteractionSource::Mouse;
	
	if (WidgetInteraction->InteractionSource == EWidgetInteractionSource::Custom)
	{
		FHitResult HitResultRight;
		if (PerformTrace(*WidgetInteraction, HitResultRight))
		{
			WidgetInteraction->SetCustomHitResult(HitResultRight);

			FVector_NetQuantize Dis = HitResultRight.TraceEnd - HitResultRight.TraceStart;
			UWidgetComponent *HoveredWidgetComponent = Cast<UWidgetComponent>(HitResultRight.GetComponent());
			bool ShowHit = false;
			if (HoveredWidgetComponent) 
			{
				Dis = HitResultRight.ImpactPoint - HitResultRight.TraceStart;
				ShowHit = true;
			}

			DrawLine(CustomMesh, Dis.Size(), ShowHit);
		}
	}
}

bool AHVRayActor::PerformTrace(UWidgetInteractionComponent &Interaction, FHitResult& HitResult)
{
	const FVector WorldLocation = Interaction.GetComponentLocation();
	const FTransform WorldTransform = Interaction.GetComponentTransform();
	const FVector Direction = WorldTransform.GetUnitAxis(EAxis::X);

	TArray<UPrimitiveComponent*> PrimitiveChildren;
	GetRelatedComponentsToIgnoreInAutomaticHitTesting(Interaction, PrimitiveChildren);

	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredComponents(PrimitiveChildren);

	TArray<struct FHitResult> CurHitResult;

	FCollisionObjectQueryParams Everything(FCollisionObjectQueryParams::AllObjects);
	if (Interaction.GetWorld()->LineTraceMultiByObjectType(CurHitResult, WorldLocation, WorldLocation + (Direction * Interaction.InteractionDistance), Everything, Params))
	{
		for (int32 i = 0; i != CurHitResult.Num(); ++i)
		{
			UWidgetComponent *HoveredWidgetComponent = Cast<UWidgetComponent>(CurHitResult[i].GetComponent());
			if (HoveredWidgetComponent && HoveredWidgetComponent->ComponentHasTag(TEXT("HVSDK")))
			{
				HitResult = CurHitResult[i];
				return true;
			}
		}

		UWidgetComponent *HoveredWidgetComponent = Cast<UWidgetComponent>(CurHitResult[0].GetComponent());
		if (HoveredWidgetComponent == NULL)
		{
			HitResult = CurHitResult[0];
			return true;
		}	
		
	}

	return false;
}

void AHVRayActor::GetRelatedComponentsToIgnoreInAutomaticHitTesting(UWidgetInteractionComponent &Interaction, TArray<UPrimitiveComponent*>& IgnorePrimitives)
{
	TArray<USceneComponent*> SceneChildren;
	if (AActor* owner = Interaction.GetOwner())
	{
		if (USceneComponent* Root = owner->GetRootComponent())
		{
			Root = Root->GetAttachmentRoot();
			Root->GetChildrenComponents(true, SceneChildren);
			SceneChildren.Add(Root);
		}
	}

	for (USceneComponent* SceneComponent : SceneChildren)
	{
		if (UPrimitiveComponent* PrimtiveComponet = Cast<UPrimitiveComponent>(SceneComponent))
		{
			// Don't ignore widget components that are siblings.
			if (SceneComponent->IsA<UWidgetComponent>())
			{
				continue;
			}

			IgnorePrimitives.Add(PrimtiveComponet);
		}
	}
}

void AHVRayActor::CheckVisible()
{
	bool vis = false;
	if (UHvInterface::GetInstance()->RayVisibility == ERayVisibility::AlwaysShow) {
		vis = true;
	}
	else if (UHvInterface::GetInstance()->RayVisibility == ERayVisibility::Auto) {
		vis = UHvInterface::GetDlgNum() > 0;
	}

	if (vis != bVisible) 
	{
		if (!vis) {
			CustomMesh->ClearCustomMeshTriangles();
		}
		bVisible = vis;
		WidgetInteraction->bEnableHitTesting = vis;
	}
}

void AHVRayActor::SetRayHand(int Index)
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepRelative, true);
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);
	WidgetInteraction->DetachFromComponent(DetachRules);
	WidgetInteraction->AttachToComponent(Index ? MotionController_Right : MotionController_Left, AttachRules);
}

void AHVRayActor::BeginPlay()
{
	Super::BeginPlay();

#if UE_VERSION_GE(4, 13)
	UWorld *world = GetWorld();
	if (world)
	{
		APlayerController* Controller = world->GetFirstPlayerController();
		EnableInput(Controller);

		FInputKeyBinding &LeftMouseButton_Pressed = InputComponent->BindKey<AHVRayActor>(EKeys::LeftMouseButton, IE_Pressed, this, &AHVRayActor::MousePress);
		LeftMouseButton_Pressed.bConsumeInput = false;

		FInputKeyBinding &LeftMouseButton_Released = InputComponent->BindKey<AHVRayActor>(EKeys::LeftMouseButton, IE_Released, this, &AHVRayActor::MouseRelease);
		LeftMouseButton_Released.bConsumeInput = false;

		FInputKeyBinding &Left_Trigger_Pressed = InputComponent->BindKey<AHVRayActor>(EKeys::MotionController_Left_Trigger, IE_Pressed, this, &AHVRayActor::MousePress);
		Left_Trigger_Pressed.bConsumeInput = false;

		FInputKeyBinding &Left_Trigger_Released = InputComponent->BindKey<AHVRayActor>(EKeys::MotionController_Left_Trigger, IE_Released, this, &AHVRayActor::MouseRelease);
		Left_Trigger_Released.bConsumeInput = false;

		FInputKeyBinding &Right_Trigger_Pressed = InputComponent->BindKey<AHVRayActor>(EKeys::MotionController_Right_Trigger, IE_Pressed, this, &AHVRayActor::MousePress);
		Right_Trigger_Pressed.bConsumeInput = false;

		FInputKeyBinding &Right_Trigger_Released = InputComponent->BindKey<AHVRayActor>(EKeys::MotionController_Right_Trigger, IE_Released, this, &AHVRayActor::MouseRelease);
		Right_Trigger_Released.bConsumeInput = false;
	}
#endif
}

void AHVRayActor::MousePress()
{
#if UE_VERSION_GE(4, 13)
	if (bVisible) {
		WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	}
#endif
}

void AHVRayActor::MouseRelease()
{
#if UE_VERSION_GE(4, 13)
	if (bVisible) {
		WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
	}
#endif
}
