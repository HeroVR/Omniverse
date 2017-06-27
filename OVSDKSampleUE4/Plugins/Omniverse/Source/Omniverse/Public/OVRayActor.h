#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OVRayActor.generated.h"

#define UE_VERSION_GE(x, y) (ENGINE_MAJOR_VERSION >= x && ENGINE_MINOR_VERSION >= y)

class UUserWidget;
class AActor;
class UWidgetInteractionComponent;
class UCustomMeshComponent;
class UMotionControllerComponent;

UCLASS()
class OMNIVERSE_API AOVRayActor : public AActor
{
	GENERATED_BODY()
public:
	AOVRayActor();
	~AOVRayActor();

	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds);

	void SetRayHand(int Index);
private:
	void DrawLine(UCustomMeshComponent *CustoemMesh, float Distance, bool ShowHit);

	bool bVisible;
	void CheckVisible();

	UMotionControllerComponent *MotionController_Left;
	UMotionControllerComponent *MotionController_Right;
	UWidgetInteractionComponent *WidgetInteraction;
	UCustomMeshComponent *CustomMesh;

	bool PerformTrace(UWidgetInteractionComponent &Interaction, FHitResult& HitResult);
	void GetRelatedComponentsToIgnoreInAutomaticHitTesting(UWidgetInteractionComponent &Interaction, TArray<UPrimitiveComponent*>& IgnorePrimitives);

	UFUNCTION()
	void MousePress();
	UFUNCTION()
	void MouseRelease();
};
