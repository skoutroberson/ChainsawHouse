// Fill out your copyright notice in the Description page of Project Settings.


#include "Flashlight.h"
#include "Grabbable.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"

AFlashlight::AFlashlight()
{
	AGrabbable::ItemGripSize = 55.f;
	AGrabbable::ValidOneHandHandHolds.SetNum(1);
	AGrabbable::ValidOneHandHandHolds.Insert(1, 0);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	ButtonPressAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("Button Press"));
	ButtonPressAudio->SetupAttachment(Mesh);

	HCRotationOffset = FRotator(-41.68f, 0.f, -90.78f);
}

void AFlashlight::BeginPlay()
{
	Super::BeginPlay();

	LM = Cast<ALightManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ALightManager::StaticClass()));
	//LM->AddFlickerLight(this, 0.34f, 5000.f, 200.f);

	// this will make cockroaches ignore the flashlight collisions
	/*
	for (auto i : GetComponents())
	{
		UPrimitiveComponent *PC = Cast<UPrimitiveComponent>(i);
		if (PC != nullptr)
		{
			PC->SetMaskFilterOnBodyInstance(1 << 3);
		}
	}
	*/
}

void AFlashlight::TurnOn()
{
	LM->TurnOn(FString("BP_Flashlight2_2"), 5000.f, 200.f);
	PlayButtonPressAudio();
	//ButtonPressAudio->Play();
	bOn = true;
}

void AFlashlight::TurnOff()
{
	LM->TurnOff(FString("BP_Flashlight2_2"), 0, 0);
	PlayButtonPressAudio();
	bOn = false;
}

void AFlashlight::PressButton(bool bButtonAudio)
{
	if (bButtonAudio)
	{
		PlayButtonPressAudio();
	}
	
	if (bOn)
	{
		LM->EditLight(this, 0, 0);
		bOn = false;
		//UE_LOG(LogTemp, Warning, TEXT("LightOff"));
		UpdateLightVolumetrics(true);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("LightOn"));
		LM->EditLight(this, 400.f, 2000.f);
		//LM->TurnOn(FString(1"BP_Flashlight2_2"), 5000.f, 200.f);
		bOn = true;
		UpdateLightVolumetrics(false);
	}
	
}

void AFlashlight::Gripped(int HandHoldNum)
{
	Super::Gripped(HandHoldNum);

	if (bSpawnFlicker)
	{
		bSpawnFlicker = false;
		//LM->RemoveFlickerLight(this);
		if (!bOn)
		{
			PressButton(false);
		}
	}


}

void AFlashlight::Released(int HandHoldNum)
{
	Super::Released(HandHoldNum);

	FlickerAfterDelay();
}
