// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Radio.h"
#include "Curves/CurveFloat.h"
#include "Door.generated.h"

UCLASS()
class CHAINSAWHOUSE_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Parameters
	UPROPERTY(EditDefaultsOnly)
	class UHapticFeedbackEffect_Base * HapticEffect;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* DoorRoot;

	class UStaticMeshComponent* DoorMesh;

	UPROPERTY(BlueprintReadOnly)
	class USceneComponent* DoorHinge;

	UPROPERTY(BlueprintReadOnly)
	class USphereComponent * Doorknob;

	// this maybe fix it? haha i think so
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipHandLocations = false;

private:
	// these are actually transforms
	USceneComponent *HandRFrontLocation = nullptr;
	USceneComponent *HandLFrontLocation = nullptr;
	USceneComponent *HandRBackLocation = nullptr;
	USceneComponent *HandLBackLocation = nullptr;

public:

	USceneComponent* GetHandRFront();
	USceneComponent* GetHandLFront();
	USceneComponent* GetHandRBack();
	USceneComponent* GetHandLBack();

	AActor* HandController = nullptr;
	FVector LastHCLocation;
	

	// Grip/Release HandController functions
	void PassController(AActor * HC);
	void SetIsBeingUsed(bool Value);


	// Mechanic functions
	void UseDoor(float DeltaTime);
	void Swing(float DeltaTime);
	void CollisionSwing(float DeltaTime);

	// State
	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	bool bIsBeingUsed = false;

	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	bool bSwing = false;

	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	float SwingVelocity;

	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	bool bCollisionSwing = false;

	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	AActor * CollisionActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = DoorMechanics)
	FVector LastCALocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	// if this bool is true, then do certain things on door close like turn off flashlight, radio, and change Errol behavior.
	UPROPERTY(EditAnywhere)
	bool bPortalRoomStartDoor = false;

	// modifiers
	UPROPERTY(EditAnywhere)
	float CloseAudioMultiplier = 10.f;

	UPROPERTY(EditAnywhere)
	float MinSwingAudioVelocity = 0.0055f;

	bool bSwingingPositive = false;
	// used for determining if we should stop the current swing audio and play another wave on direction changes.
	bool bSwingingDirectionChange = false;

private:

	class UWorld *World;
	
	float SlerpSize;
	const float HingeFriction = 0.3f;
	const float DoorLength = 91.f;
	float MaxAngleRadians = 0.0f;
	float MinAngleRadians = 0.0f;

	bool KnobCollision = false;

	int Push = 0;
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FQuat MinRotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FQuat MaxRotation;

private:
	// Helpers

	FVector2D ConvertVector3D(FVector Vec);
	FQuat CalcGoalQuat(FVector GoalVec);
	float BinarySearchForMaxAngle();

	// Functions to be called in Stage classes!
public:
	// bool for not letting the player use the door if the stage closes/is closing it 
	UPROPERTY(BlueprintReadWrite)
	bool bCloseDoorFast = false;

	UFUNCTION(BlueprintCallable)
	void CloseDoorFast(UPARAM(DisplayName = "DeltaTime") float DeltaTime);


private:
	bool bStageLock = false;
	float CloseDoorFastVelocity = 0.0001f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bLocked = false;

private:
	float YawAngle = -1.f;
	int DoorCloseDirection = 0;

private:
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * OpenSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * CloseSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * SwingOpenSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * SwingCloseSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * LockedSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * UnlockSound;
	UPROPERTY(EditDefaultsOnly)
	class USoundCue * HandCollisionSound;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue * CapsuleCollisionSound;

	bool bSwingingOpen = false;
	bool bPlayingSwingSound = false;

	float DoorVelocitySoundThreshold = 0.01f;
	
	float SwingOpenSoundDuration = 0;
	float SwingCloseSoundDuration = 0;

	void PlaySwingSound(const float Velocity, const float Ratio);

	void PlaySwingAudio(const float Velocity);

	float MaxSwingVelocity = 0;

public:

	UPROPERTY(BlueprintReadWrite)
	UAudioComponent * SwingAudioComponent = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UAudioComponent *OpenHitAudio = nullptr;

	// A key with this tag will unlock this door.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FName KeyTag;

	UFUNCTION(BlueprintCallable)
	void UnlockDoor(FName KeyName);

	// Set to true on component start overlap, false on end overlap.
	UPROPERTY(BlueprintReadWrite)
	bool bColliding = false;

	// this needs to be manually set to false if the door is open at the start
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bFullyClosed = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bBackwards = false;

	UFUNCTION(BlueprintCallable)
	void SetDoorBackwards();

	// this will be used to do certain things when the portal door is closed. (turn off sounds/lighting)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DoorMechanics)
	bool bPortalEnabled = false;

	// This represents the angle the door is away from being fully closed. 0 = fully closed.
	UPROPERTY(BlueprintReadOnly)
	float CurrentDoorAngle = 0.0f;

	UPROPERTY(EditAnywhere)
	UCurveFloat * DoorCloseCurve1 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	UCurveFloat * CurrentCurve = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bCloseDoorUsingCurve = false;

	UPROPERTY(BlueprintReadWrite)
	bool bOpenDoorUsingCurve = false;

	UPROPERTY(BlueprintReadWrite)
	float CurrentCurveTime = 0.0f;

	UFUNCTION(BlueprintCallable)
	void CloseDoorUsingCurve(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void OpenDoorUsingCurve(float DeltaTime);

	// used by portal room doors to turn off outside sounds and indirect lighting
	UFUNCTION(BlueprintImplementableEvent)
	void TurnOffSoundsAndLighting();

	UFUNCTION(BlueprintImplementableEvent)
	void TurnOnSoundsAndLighting();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsErrolOpening = false;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ResetIsErrolOpening(float Duration);

	void InterpToHC(float DeltaTime);
	FVector InterpHCLocation = FVector::ZeroVector;
	FVector LastInterpHCLocation = FVector::ZeroVector;

	// for testing with Click() in VRCharacter.cpp
	UFUNCTION(BlueprintImplementableEvent)
	void UnlockAnimationCPP();

	//offsets from MinRotation and MaxRotation in radians where we will play swing audio (like real door hinge squeaks i think)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SqueakMinStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SqueakMinEnd = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SqueakMidPoint = 0.78f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SqueakRadius = 0.4f;

	// cannot be larger than PI/2 - SqueakRadius
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SqueakOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayingCloseSound = false;

	UFUNCTION(BlueprintImplementableEvent)
	void CloseSoundDelay(float CloseVolume = 1.0f);

	UFUNCTION(BlueprintImplementableEvent)
	void ClickUnlockDoor();
};
