// Fill out your copyright notice in the Description page of Project Settings.


#include "Chainsaw.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "HandController.h"
#include "Math/UnrealMathUtility.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"


AChainsaw::AChainsaw()
{
	AGrabbable::ItemGripSize = 58.f;
	AGrabbable::bTwoHanded = true;
	AGrabbable::ValidOneHandHandHolds.SetNum(1);
	AGrabbable::ValidOneHandHandHolds.Insert(2, 0);
	PrimaryActorTick.bCanEverTick = true;

	//StartupAudio->OnAudioFinished.AddDynamic(this, &AChainsaw::ExitStartupState);
	//EndrevAudio->OnAudioFinished.AddDynamic(this, &AChainsaw::ExitEndrevState);
}

void AChainsaw::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<AVRCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), AVRCharacter::StaticClass()));

	UActorComponent * AC = GetComponentByClass(UBoxComponent::StaticClass());
	BladeCollision = Cast<UBoxComponent>(AC);

	if (BladeCollision == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BladeCollision cast failed in Chainsaw.cpp"));
	}
	else
	{
		BladeOffset = GetActorLocation() - BladeCollision->GetComponentLocation();
	}

	// THESE TWO LINES NEED TO BE IN EVERY TWO HANDED GRABBABLE ACTOR
	AGrabbable::HandHold1 = Cast<USceneComponent>(GetComponentsByTag(UActorComponent::StaticClass(), TEXT("1"))[0]);
	AGrabbable::HandHold2 = Cast<USceneComponent>(GetComponentsByTag(UActorComponent::StaticClass(), TEXT("2"))[0]);

	SkeletalMesh = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	SetRootComponent(SkeletalMesh);

	EngineAudio = Cast<UAudioComponent>(GetComponentsByTag(UAudioComponent::StaticClass(), FName("rev"))[0]);

	EngineAudio->SetFloatParameter(FName("RPM"), 0.0f);
	EngineAudio->SetBoolParameter(FName("CD"), false);

	//BladeCollision->OnComponentBeginOverlap.AddDynamic(this, &AChainsaw::BladeBeginOverlap);

	EnterIdleState();
}

void AChainsaw::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*
	if (b1Held)
	{
		//TriggerAxisUpdates(DeltaTime);
		//UE_LOG(LogTemp, Warning, TEXT("%f"), LastTriggerAxisValue);
	}*/

	if (bRandomShake)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Shake"));
		RandomShake(DeltaTime);
	}
	
	if (bDismembering)
	{
		TickDismember(DeltaTime);
		// cut animation until the saw is all the way through
		return;
	}
	else if (b1Held && MotionController1 != nullptr) // release hand hold 1 if the player moves too far away from the saw
	{
		if (FVector::Dist(GetActorLocation(), MotionController1->GetActorLocation()) > 200.f)
		{
			AHandController *c = Cast<AHandController>(MotionController1);
			c->Release();
		}
	}



	/*
	switch (State)
	{
	case SawState::STATE_IDLE:
		// check if TriggerValue is greater than 0.5f, if so then change state to Startup
		TickIdleState(DeltaTime);
		break;
	case SawState::STATE_STARTUP:
		// play sound, startup blade, vibration, and shake. when the sound is completed, change state to Revving
		TickStartupState(DeltaTime);
		break;
	case SawState::STATE_REVVING:
		// loop rev sound, blade spin, vibration, and shake. check if CurrentEngineValue is less than 0.5f, if so then change state to Endrev
		TickRevvingState(DeltaTime);
		break;
	case SawState::STATE_ENDREV:
		// play end rev, end blade spin, vibration and shake, when sound ends, change state to Idle
		TickEndrevState(DeltaTime);
		break;
	}
	*/
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + SkeletalMesh->GetRightVector() * 100.f, FColor::Red, false, 2 * DeltaTime);
	/*
	if (AGrabbable::bInterpToMC)
	{
		InterpToMC(DeltaTime);
	}
	if (AGrabbable::bRotateTwoHand)
	{
		RotateTwoHand(DeltaTime);
	}
	else if (AGrabbable::bRotateOneHand)
	{
		RotateOneHand(DeltaTime);
	}
	*/
}

void AChainsaw::TriggerAxisUpdates(float DeltaTime)
{
	// update pitch of engine
	// update chain rotation speed
	// update shake value
	// update haptic intensity? (maybe not this one)

	// 0.0f -> 1.0f
	const float TriggerAxisValue = (bLeftHandIsControllingTrigger) ? Player->LeftTriggerAxisValue : Player->RightTriggerAxisValue;

	// this is used for the current engine audio.
	float NewRPMValue = TriggerAxisValue;

	if(TriggerAxisValue < CurrentEngineValue)
	{
		NewRPMValue += 2.0f;
	}

	//EngineAudio->SetPitchMultiplier(TriggerAxisValue * 2.f);
	CurrentEngineValue = TriggerAxisValue;
	/*
	if (TriggerAxisValue > LastTriggerAxisValue)
	{
		EngineAudio->SetBoolParameter(FName("CD"), false);
	}
	else if(TriggerAxisValue < LastTriggerAxisValue)
	{
		EngineAudio->SetBoolParameter(FName("CD"), true);
	}
	*/
	// instead of doing this. I think I want a seperate function to increase/decrease EngineValue based on curves
	UE_LOG(LogTemp, Warning, TEXT("RPM: %f"), TriggerAxisValue);
	EngineAudio->SetFloatParameter(FName("RPM"), TriggerAxisValue);
	
	/*
	if (TriggerAxisValue > 0.1f)
	{
		Heat += HeatUpSpeed * (1/Heat) * DeltaTime;
		if (Heat > MaxHeat)	// clamp
		{
			Heat = MaxHeat;
		}
	}
	else
	{
		if (Heat > MaxHeat * 0.5f)
		{
			Heat -= CooldownSpeed * (1 / Heat) * DeltaTime;
		}
		else
		{
			Heat -= CooldownSpeed * Heat * 0.2f * DeltaTime;
		}


		if (Heat < 0) // clamp
		{
			Heat = 0;
		}
	}

	//EngineAudio->SetFloatParameter(FName("TAVP"), TriggerAxisValue);
	//EngineAudio->SetFloatParameter(FName("TAVV"), TriggerAxisValue);

	
	if (TriggerAxisValue > LastTriggerAxisValue || TriggerAxisValue >= 0.9f)
	{
		CurrentEngineValue += HeatUpSpeed * Heat * (1 / TriggerAxisValue);

		if (CurrentEngineValue > MaxEngineValue)
		{
			CurrentEngineValue = MaxEngineValue;
		}

		EngineAudio->SetFloatParameter(FName("TAVP"), CurrentEngineValue);
		EngineAudio->SetFloatParameter(FName("TAVV"), CurrentEngineValue);
		if (CurrentEngineValue > 0.7f)
		{
			EngineAudio->SetVolumeMultiplier(CurrentEngineValue);
		}
	}
	else if(TriggerAxisValue < LastTriggerAxisValue || TriggerAxisValue < 0.9f)
	{
		if (CurrentEngineValue > 0.9f)
		{
			CurrentEngineValue -= CooldownSpeed * CooldownSpeed * CooldownSpeed * (1 / (Heat * CurrentEngineValue)) * 18.f * DeltaTime;
		}
		else
		{
			if (Heat > 0.1f)
			{
				CurrentEngineValue -= CooldownSpeed * Heat * CurrentEngineValue * 0.8f * DeltaTime;
			}
			else
			{
				CurrentEngineValue -= CooldownSpeed * DeltaTime * 0.5f;
			}
			
		}

		if (CurrentEngineValue < 0) // clamp
		{
			CurrentEngineValue = 0;
		}

		if (CurrentEngineValue > 0)
		{
			EngineAudio->SetFloatParameter(FName("TAVP"), CurrentEngineValue);
			EngineAudio->SetFloatParameter(FName("TAVV"), CurrentEngineValue);

			if (CurrentEngineValue > 0.7f)
			{
				EngineAudio->SetVolumeMultiplier(CurrentEngineValue);
			}
			
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Heat: %f"), Heat);
	*/

	LastTriggerAxisValue = TriggerAxisValue;
}

void AChainsaw::RandomShake(float DeltaTime)	//	calling this every frame is not ideal?
{
	float ShakeIntensity = RevShakeMaxIntensity;
	float x = FMath::RandRange(-ShakeIntensity, ShakeIntensity);
	float y =  FMath::RandRange(-ShakeIntensity, ShakeIntensity);
	float z = FMath::RandRange(-ShakeIntensity, ShakeIntensity);
	FRotator ShakeRotator = FRotator(x, y, z);
	SkeletalMesh->AddRelativeRotation(ShakeRotator);
	//AddActorWorldRotation(ShakeRotator, false, nullptr, ETeleportType::TeleportPhysics);
}

void AChainsaw::StopShake()
{
	bRandomShake = false;
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AChainsaw::SpawnBlood(float DeltaTime)
{
	BloodSpawnCounter += DeltaTime;

	if (BloodSpawnCounter > BloodSpawnTime)
	{
		BloodSpawnCounter = 0;

		FActorSpawnParameters SpawnParams;

		World->SpawnActor<AActor>(BloodFXActor, BladeCollision->GetComponentTransform());
	}
}

void AChainsaw::StartDismember()
{
	// move saw to the starting location
	// rotate saw correctly
	// start blood spray VFX
	// start dismbember tick

	// rotate right vector to equal the dismember up vector (arrow component in ErrolCharacter)

	bInterpToMC = false;
	bRotateTwoHand = false;
	bRotateOneHand = false;
	
	const AHandController * HC = Cast<AHandController>(NonControllingMC);
	const FVector CO = HC->ChainsawOffset->GetComponentLocation();
	const FVector MCDif = (CO - ControllingMC->GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(MCDif, CutVectors->GetForwardVector());
	float RotationAngle = acosf(DotProduct) * (PI/180.f);

	FRotator NewRotation = CutVectors->GetComponentRotation();
	//NewRotation.Pitch = MCDif.Rotation().Pitch;

	SetActorRotation(NewRotation);

	BladeOffset = GetActorLocation() - BladeCollision->GetComponentLocation();
	SetActorLocation(CutStartLocation->GetComponentLocation() + BladeOffset);
	

	/*
	FVector RightVector = GetActorRightVector();
	FVector RotationAxis = FVector::CrossProduct(RightVector, CutUpVector);
	RotationAxis.Normalize();
	float DotProduct = FVector::DotProduct(RightVector, CutUpVector);ft
	float RotationAngle = acosf(DotProduct);
	FQuat RotQuat(RotationAxis, RotationAngle);
	FQuat ActorQuat = GetActorQuat();
	FQuat NewQuat = RotQuat * ActorQuat;
	
	SetActorRotation(NewQuat);
	*/

	bDismembering = true;
	
}

void AChainsaw::EndDismember()
{
	// move saw back to player control
	// ragdoll top and bottom half of body
	// update current stage perhaps? maybe end the game?

	ErrolMainMesh->SetVisibility(false);
	ErrolTopHalfMesh->SetVisibility(true);
	ErrolBottomHalfMesh->SetVisibility(true);
	//ErrolChainsawMesh->SetSimulatePhysics(true);
	ErrolTopHalfMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	ErrolBottomHalfMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	ErrolTopHalfMesh->SetSimulatePhysics(true);
	ErrolBottomHalfMesh->SetSimulatePhysics(true);

	ErrolChar->Dismembered();

	// spawn dog out of view
	// spawn blood on each half mesh

	bDismembering = false;
	
	ControllingHandController->Release();
	//ControllingHandController->Sister

	AHandController *NCHC = Cast<AHandController>(NonControllingMC);

	if (NCHC != nullptr && NCHC->bIsHoldingChainsaw)
	{
		NCHC->Release();
	}

	if (ControllingHandController != nullptr && ControllingHandController->bIsHoldingChainsaw)
	{
		ControllingHandController->Release();
	}

	//LeftC

	StopHaptics();

	// this will cause some issues if the player lets go of the chainsaw while dismembering... I need to think of a way to deal with this

	/*
	if (ControllingHandController != nullptr)
	{
		if (ControllingHandController->bIsHoldingChainsaw)
		{
			bInterpToMC = true;
			bRotateTwoHand = true;
		}
	}
	*/
}

void AChainsaw::TickDismember(float DeltaTime)
{
	// move saw along dismember path until the saw is all the way through.
	// check the saw trigger and hand locations to see if the dismember should stop early
	// check if the saw is finished cutting and then end dismember

	// rotation stuff
	const AHandController * HC = Cast<AHandController>(NonControllingMC);
	const FVector CO = HC->ChainsawOffset->GetComponentLocation();
	const FVector MCDif = (CO - ControllingMC->GetActorLocation()).GetSafeNormal();

	const FRotator MCRot = MCDif.Rotation();

	const FRotator SawRotation = GetActorRotation();

	FRotator NewRotation(MCRot.Pitch, SawRotation.Yaw, SawRotation.Roll);

	const float MaxPitch = 10.f;
	const float MinPitch = -40.f;
	const float HalfPitch = 25.f;

	float PitchDiff = MCRot.Pitch - SawRotation.Pitch;
	float PitchCheck = CutVectors->GetForwardVector().Rotation().Pitch - GetActorRotation().Pitch;

	if (PitchDiff < 0.0f && PitchCheck < MaxPitch || PitchDiff > 0.0f && PitchCheck > MinPitch)
	{
		// this next piece of code will smooth out the rotation so it doesn't suddenly stop rotating when the saw rotation is at max/min
		// but instead slowly eases to a stop.

		float SlowDownMultiplier = 0.5f;

		if (PitchCheck >= (MaxPitch + MinPitch) * 0.5f && PitchDiff < 0)
		{
			SlowDownMultiplier = ((MaxPitch + 180.f) - (PitchCheck + 180.f)) * 0.01f;
		}
		else if(PitchCheck <= (MaxPitch + MinPitch) * 0.5f && PitchDiff > 0)
		{
			SlowDownMultiplier = ((PitchCheck + 180.f) - (MinPitch + 180.f)) * 0.01f;
		}
		
		SlowDownMultiplier = (SlowDownMultiplier < 0.001f) ? 0.001f : SlowDownMultiplier;

		//GEngine->AddOnScreenDebugMessage(-1, DeltaTime * 1.1f, FColor::Magenta, FString::Printf(TEXT("Slow: %f"), SlowDownMultiplier));

		float TotalPitchChange = PitchDiff * DismemberRotateSpeedModifier * SlowDownMultiplier;

		AddActorLocalRotation(FRotator(TotalPitchChange, 0, 0));
	}

	const FVector EndLocation = CutEndLocation->GetComponentLocation();
	const FVector EndLocationPlusOffset = EndLocation + BladeOffset;
	const FVector BladeLocation = BladeCollision->GetComponentLocation();

	BladeOffset = GetActorLocation() - BladeLocation;
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), EndLocationPlusOffset, DeltaTime, DismemberCutSpeed));
	//SetActorLocation(CutStartLocation->GetComponentLocation() + BladeOffset);


	// check if cut is finished, if so then end dismember

	const float DistanceToEnd = FVector::DistSquared(EndLocation, BladeLocation);

	if (DistanceToEnd < 110.0f)
	{
		EndDismember();
	}

	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime * 1.1f, FColor::Cyan, FString::Printf(TEXT("Distance: %f"), DistanceToEnd));
	/*
	FVector MinCutV = MinCutVector->GetForwardVector();
	FVector MaxCutV = MaxCutVector->GetForwardVector();

	float MinDot = FVector::DotProduct(GetActorForwardVector(), MinCutV);
	float MaxDot = FVector::DotProduct(GetActorForwardVector(), MaxCutV);
	*/
	
	//DrawDebugLine(GetWorld(), ControllingMC->GetActorLocation(), ControllingMC->GetActorLocation() + MCDif * 100.f, FColor::Green, false, DeltaTime * 1.1f);
	
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime * 1.1f, FColor::Cyan, FString::Printf(TEXT("PitchDiff: %f"), PitchDiff));
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime * 1.1f, FColor::Green, FString::Printf(TEXT("PitchCheck: %f"), PitchCheck));

	//AddActorLocalOffset(BladeOffset);

	// spawn blood:
	SpawnBlood(DeltaTime);
}

void AChainsaw::Gripped(int HandHoldNum)
{
	if (!bDismembering)
	{
		Super::Gripped(HandHoldNum);
	}
	
	if (HandHoldNum == 1)
	{
		const AHandController * HC = Cast<AHandController>(MotionController1);

		if (HC->bLeft)
		{
			bLeftHandIsControllingTrigger = true;
		}
		else
		{
			bLeftHandIsControllingTrigger = false;
		}
	}
}

void AChainsaw::Released(int HandHoldNum)
{
	if (!bDismembering)
	{
		Super::Released(HandHoldNum);
	}
	
	if (CurrentEngineValue > 0.0f)
	{
		// interpolate value to 0. Cancel interp if picked up again
	}
}

void AChainsaw::InterpToMC(float DeltaTime)
{
	Super::InterpToMC(DeltaTime);
	/*
	const FVector AL = GetActorLocation();
	ControllingOffset = GetActorLocation() - ControllingHandHold->GetComponentLocation();
	const FVector TL = AGrabbable::ControllingMC->GetActorLocation() + ControllingOffset;
	SetActorLocation(UKismetMathLibrary::VInterpTo_Constant(AL, TL, DeltaTime, 300.f));
	*/
}

void AChainsaw::RotateOneHand(float DeltaTime)
{
	Super::RotateOneHand(DeltaTime);
	/*
	// this only works for the chainsaw
	FRotator MCRot = ControllingMC->GetActorRotation();
	MCRot.Roll = (bControllingMCLeft) ? MCRot.Roll - 90.f : MCRot.Roll += 90.f;
	SetActorRotation(UKismetMathLibrary::RLerp(GetActorRotation(), MCRot, 5.f * DeltaTime, true));
	*/
}

void AChainsaw::RotateTwoHand(float DeltaTime)
{
	Super::RotateTwoHand(DeltaTime);
	/*
	const AHandController * HC = Cast<AHandController>(NonControllingMC);
	const FVector CO = HC->ChainsawOffset->GetComponentLocation();
	const FVector MCDif = (CO - ControllingMC->GetActorLocation()).GetSafeNormal();
	FRotator NewRot = MCDif.ToOrientationRotator();
	NewRot.Roll = ControllingMC->GetActorRotation().Roll;

	SetActorRotation(UKismetMathLibrary::RLerp(GetActorRotation(), NewRot, 5.f * DeltaTime, true));

	/*
	const FVector SMFV = SkeletalMesh->GetForwardVector();
	//const FVector MC1L = AGrabbable::MotionController1->GetActorLocation();
	const FVector MC1L = AGrabbable::MC1OffsetComponent->GetComponentLocation();
	const FVector MC2L = AGrabbable::MotionController2->GetActorLocation();
	DrawDebugSphere(GetWorld(), MC1L, 5.f, 10.f, FColor::Cyan, false, 2 * DeltaTime);

	float D = FVector::Distance(MC1L, MC2L);
	if (D > TwoHandDistance + TwoHandDropThreshold || D < TwoHandDistance - TwoHandDropThreshold)
	{
		Cast<AHandController>(AGrabbable::MotionController1)->Release();
		Cast<AHandController>(AGrabbable::MotionController2)->Grip();
		return;
	}

	const FVector MCDif = (MC1L - MC2L).GetSafeNormal();
	const float Angle = FMath::Acos(FVector::DotProduct(SMFV, MCDif));
	const FVector Axis = FVector::CrossProduct(SMFV, MCDif).GetSafeNormal();
	const FQuat DeltaRotation = FQuat(Axis, Angle);
	const FQuat NewRotation = DeltaRotation * GetActorQuat();
	const FQuat SlerpQuat = FQuat::Slerp(GetActorQuat(), NewRotation, 3.5f * DeltaTime);
	SetActorRotation(SlerpQuat);

	 
	const float Angle2 = FMath::FMath::Acos(FVector::DotProduct(SkeletalMesh->GetRightVector(), AGrabbable::MotionController1->GetActorRightVector()));
	const FVector Axis2 = FVector::CrossProduct(SkeletalMesh->GetRightVector(), AGrabbable::MotionController1->GetActorRightVector()).GetSafeNormal();
	const FQuat DeltaRot = FQuat(Axis2, Angle2);
	const FQuat NewRot = DeltaRot * GetActorQuat();
	const FQuat SlerpQuat2 = FQuat::Slerp(GetActorQuat(), NewRot, 3.f * DeltaTime);
	SetActorRotation(SlerpQuat2);
	*/
}

void AChainsaw::BladeBeginOverlap(UPrimitiveComponent * FirstComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap Errol!"));
	DrawDebugSphere(GetWorld(), SweepResult.ImpactPoint, 5.f, 10.f, FColor::Cyan, true);
}

void AChainsaw::EnterIdleState()
{
	// play looping idle sound
	State = SawState::STATE_IDLE;
	/*
	if (!IdleAudio->IsPlaying())
	{
		IdleAudio->Play();
	}
	*/
	//RevvingAudio->SetIntParameter(FName("State"), 1);
}

void AChainsaw::EnterStartupState()
{
	// play startup sound
	State = SawState::STATE_STARTUP;
	//StartupAudio->Play();
	//RevvingAudio->SetIntParameter(FName("State"), 0);
}

void AChainsaw::EnterRevvingState()
{
	// play looping rev sound
	State = SawState::STATE_REVVING;

	/*
	if (!RevvingAudio->IsPlaying())
	{
		RevvingAudio->Play();
	}
	*/
	//RevvingAudio->SetIntParameter(FName("State"), 2);
}

void AChainsaw::EnterEndrevState()
{
	// play end rev sound
	State = SawState::STATE_ENDREV;
	//EndrevAudio->Play();
	//RevvingAudio->SetIntParameter(FName("State"), 0);
}

void AChainsaw::TickIdleState(float DeltaTime)
{
	// check if TriggerValue is greater than 0.5f, if so then change state to Startup
	if (CurrentEngineValue > 0.5f)
	{
		ExitIdleState();
		EnterStartupState();
	}
}

void AChainsaw::TickStartupState(float DeltaTime)
{
	// play sound, startup blade, vibration, and shake. when the sound is completed, change state to Revving
	
	// maybe check if CurrentEngineValue goes below 0.5f again, if so then go to EndrevState and play the endrev sound at the same time that this sound finished at.
}

void AChainsaw::TickRevvingState(float DeltaTime)
{
	// loop rev sound, blade spin, vibration, and shake. check if CurrentEngineValue is less than 0.5f, if so then change state to Endrev
	if (CurrentEngineValue < 0.5f)
	{
		ExitRevvingState();
		EnterEndrevState();
	}
}

void AChainsaw::TickEndrevState(float DeltaTime)
{
	// play end rev, end blade spin, vibration and shake, when sound ends, change state to Idle
}

void AChainsaw::ExitIdleState()
{
	//IdleAudio->Stop();
}

void AChainsaw::ExitStartupState()
{
	UE_LOG(LogTemp, Warning, TEXT("end startup!!!"));
	EnterRevvingState();
}

void AChainsaw::ExitRevvingState()
{
	//RevvingAudio->Stop();
}

void AChainsaw::ExitEndrevState()
{
	EnterIdleState();
}
