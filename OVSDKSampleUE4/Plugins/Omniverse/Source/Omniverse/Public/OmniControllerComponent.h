// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "OmniControllerPluginFunctionLibrary.h"
#include "OmniControllerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInitOmniDone);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OMNIVERSE_API UOmniControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOmniControllerComponent();

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForceDeveloperMode;

	UPROPERTY(BlueprintReadWrite)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CameraTag;

	UPROPERTY(BlueprintAssignable, Category = "OmniControllerComponent")
	FInitOmniDone InitOmniDone;

	UPROPERTY(BlueprintReadOnly)
	float CouplingPercentage;		// 0: use harness dir(Decouple mode); 1: HMD dir(Couple mode); 0~1: blend between harness and HMD (SoftCouple mode);

	UPROPERTY(BlueprintReadWrite)
	float OmniYawOffset;			// Calibrated yaw between Omni-Device and Game-World coordination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoCorrectStartYaw;		// Auto-correct harness to ACharacter's orientation on start;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMin="0.0", UIMax="1.0"))
	float BackwardMovementMod;		//With the CBM (Cadence Based Movement), the Backwards movement speed is very fast
									//Set this value to slow down Backwards Movement. [0,1]

	UFUNCTION(BlueprintCallable, Category = "OmniControllerComponent")
	bool IsDeveloperMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OmniControllerComponent")
		float GetRawOmniYaw() { return RawOmniYaw; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OmniControllerComponent")
		float GetAdjustedOmniYaw() { return OmniYaw; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OmniControllerComponent")
		float GetCurrentMovementYaw() { return CurrYaw; };

	FRotator GetCurrentMovementDirection() { return MovementDirection; };

protected:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EOmniState::Type> OmniState;

	void CheckOmniState();


protected:
	float RawOmniYaw;				// Harness Yaw in Omni-Device coordination;	
	float OmniYaw;					// Harness Yaw in Game-world coordination;

	bool bStartYawCorrected;
	float StartYawDiff;				// Yaw difference between PlayerStart and harness;

	float CurrYaw;					// Owner current orientation;
	
	bool StartYawSet;				// Says whether or not the Start Yaw has been set. Makes sure the character is facing Forward.
							// Gets set in the TickComponent(...) function.

	FRotator MovementDirection;
};
