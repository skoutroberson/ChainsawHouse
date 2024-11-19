// Fill out your copyright notice in the Description page of Project Settings.


#include "Radio.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "VRCharacter.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"

// Sets default values
ARadio::ARadio()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SoundLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SoundLocation"));
	SoundLocation->SetupAttachment(Root);

	PathStart = CreateDefaultSubobject<USceneComponent>(TEXT("PathStart"));
	PathStart->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void ARadio::BeginPlay()
{
	Super::BeginPlay();
	
	World = GetWorld();
	NavigationSystem = UNavigationSystemV1::GetCurrent(World);

	Player = UGameplayStatics::GetActorOfClass(World, AVRCharacter::StaticClass());
	PlayerCamera = Cast<USceneComponent>(Player->GetComponentByClass(UCameraComponent::StaticClass()));

	AC = Cast<UAudioComponent>(GetComponentsByTag(UAudioComponent::StaticClass(), FName("VoiceAudio"))[0]);
	Att = AC->Sound->AttenuationSettings;

	LastPlayerLocation = Player->GetActorLocation();

	GetNewPath();
}

// Called every frame
void ARadio::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// this should be called on a timer and not every frame
	//UpdateSoundLocation(DeltaTime);

	SimulateReverb(DeltaTime);
}


void ARadio::UpdateSoundLocation(float DeltaTime)
{
	const FVector RL = PathStart->GetComponentLocation();
	const FVector PL = Player->GetActorLocation();
	const FVector SL = SoundLocation->GetComponentLocation();
	const FVector AL = GetActorLocation();

	const FVector CL = PlayerCamera->GetComponentLocation();
	const FVector CFV = PlayerCamera->GetForwardVector();

	const FVector disp = CL - AL;
	const float dist = disp.Size();

	FVector NewSoundLocation = AL;

	//UNavigationPath * Path = NavigationSystem->FindPathToLocationSynchronously(World, PL, RL);
	//TArray<FVector> PathPoints = Path->PathPoints;

	// only get a new path every time the player moves 50uu 
	if (FVector::Dist(PL, LastPlayerLocation) > 50.f)
	{
		LastPlayerLocation = PL;
		GetNewPath();
	}

	//const int n = PathPoints.Num();
	const int n = NavPathPoints.Num();

	CurrentPathPointsNum = n;

	if (n > 3)
	{
		

		const float g = FMath::Clamp(1.0f / FMath::Clamp(n, 1, 5) * 1.75f, 0.1f, 1.0f);

		FVector p = NavPathPoints[2];
		const float d = FVector::Dist2D(PL, p);
		if (d < 200.f)
		{
			p = NavPathPoints[3];
			//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, g * 2.0f, DeltaTime, 0.3f);
		}

		//DrawDebugPoint(World, PathPoints[1], 20.f, FColor::Cyan, false, DeltaTime * 1.1f);

		const FVector CDisp = (AL - CL).GetSafeNormal();
		const float Dot = FVector::DotProduct(CDisp, CFV);

		if (n > 6)
		{
			p = NavPathPoints[n - 4];
		}
		
		p.Z += 100.f;

		/*
		if (Dot < 0.98f)
		{*/
		NewSoundLocation = p;
			//CurrentStereoSpread = FMath::FInterpConstantTo(CurrentStereoSpread, 3000.f, DeltaTime, 0.3f);
		//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, g, DeltaTime, 0.3f);
		/*}
		else if(dist < 800.f)
		{
			//CurrentStereoSpread = FMath::FInterpConstantTo(CurrentStereoSpread, 400.f, DeltaTime, 0.3f);
			CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, g * 2.0f, DeltaTime, 0.3f);
		}*/

		if (Dot > 0.97f && dist < 710.f)
		{
			//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, g * 2.0f, DeltaTime, 0.3f);
			NewSoundLocation = AL;
		}

		CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, DistanceFadeCurve->GetFloatValue(dist) * g, DeltaTime, 0.3f);
		//SoundLocation->SetWorldLocation(FMath::VInterpConstantTo(SL, p, DeltaTime, 165.0f));
		//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, g, DeltaTime, 0.3f);
	}
	else
	{
		//SoundLocation->SetWorldLocation(FMath::VInterpConstantTo(SL, GetActorLocation(), DeltaTime, 165.0f));
		CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, 1.0f, DeltaTime, 0.3f);
		//CurrentStereoSpread = FMath::FInterpConstantTo(CurrentStereoSpread, 400.f, DeltaTime, 0.3f);
	}

	SoundLocation->SetWorldLocation(FMath::VInterpConstantTo(SL, NewSoundLocation, DeltaTime, 800.0f));
	AC->SetVolumeMultiplier(CurrentAudioVolume);
	//Att->Attenuation.StereoSpread = CurrentStereoSpread;
	//AC->AttenuationSettings->Attenuation.StereoSpread = CurrentStereoSpread;

	UE_LOG(LogTemp, Warning, TEXT("AudioVolume: %f"), CurrentAudioVolume);
}

void ARadio::SimulateReverb(float DeltaTime)
{
	const FVector RL = PathStart->GetComponentLocation();
	const FVector PL = Player->GetActorLocation();
	const FVector SL = SoundLocation->GetComponentLocation();
	const FVector AL = GetActorLocation();

	const FVector CL = PlayerCamera->GetComponentLocation();
	const FVector CFV = PlayerCamera->GetForwardVector();

	const FVector disp = CL - AL;
	const float dist = disp.Size();

	float NewAudioVolume = DistanceFadeCurve->GetFloatValue(dist);
	float NewAudioMultiplier = 1.0f;

	FVector NewSoundLocation = AL;

	float NewReverbValue = 0.0f;

	// only get a new path every time the player moves 50uu 
	if (FVector::Dist(PL, LastPlayerLocation) > 50.f)
	{
		LastPlayerLocation = PL;
		GetNewPath();
	}

	//UNavigationPath * Path = NavigationSystem->FindPathToLocationSynchronously(World, PL, RL);
	//TArray<FVector> PathPoints = Path->PathPoints;

	const int n = NavPathPoints.Num();
	CurrentPathPointsNum = n;

	if (n > 3 && n < 7)
	{
		const float g = FMath::Clamp(1.0f / FMath::Clamp(n, 1, 5) * 2.0f, 0.01f, 1.0f);

		FVector p = NavPathPoints[2];

		/*
		for (int i = 0; i < n; ++i)
		{
			DrawDebugPoint(World, PathPoints[i], 10.f, FColor::Magenta, false, DeltaTime * 1.1f);
		}
		*/

		const FVector CDisp = (AL - CL).GetSafeNormal();
		const float Dot = FVector::DotProduct(CDisp, CFV);

		if (dist > ReverbDistance)
		{
			if (Dot > 0.96f && dist < 800.f)
			{
				//NewSoundLocation = CL + CDisp * ReverbDistance;
				//NewAudioMultiplier = 0.12f;
				//NewAudioMultiplier = 0.35f;
				NewAudioMultiplier = 0.45f;
			}
			/*else
			{*/
				float PathDistance = 0.0f;
				int Index = 0;
				float PPDist = 0;

				FVector P1;
				FVector P2;

				FVector PLNav = PL;
				PLNav.Z = NavPathPoints[0].Z;
				//NavPathPoints[0] == PLNav; // this doesn't work. need to do RemoveAt, and then insert. I coudn't figure out why
				// the sound location wasn't updating when I changed the nav path to only update when the player moved a certain distance.
				// this change fixed it. Weird bug...

				NavPathPoints.RemoveAt(0);
				NavPathPoints.Insert(PLNav, 0);

				//UE_LOG(LogTemp, Warning, TEXT("----"));
				while (PathDistance < ReverbDistance && Index < n - 1)
				{
					const FVector P_1 = NavPathPoints[Index];
					const FVector P_2 = NavPathPoints[Index + 1];

					const float d = FVector::Dist(P_1, P_2);

					//UE_LOG(LogTemp, Warning, TEXT("d: %f"), d);

					const float DSum = PathDistance + d;

					if (DSum >= ReverbDistance)
					{
						PPDist = d;
						P1 = P_1;
						P2 = P_2;
						break;
					}

					++Index;
					PathDistance = DSum;
				}
				//UE_LOG(LogTemp, Warning, TEXT("----"));
				
				const float RemainingDist = ReverbDistance - PathDistance;
				const FVector PPDisp = P2 - P1;
				const FVector PPDir = PPDisp.GetSafeNormal();
				FVector NewLocation = P1 + PPDir * RemainingDist;
				NewLocation.Z += 150.f;
				NewSoundLocation = NewLocation;
				//NewLocation -= PPDir * 50.f;
				/*
				// reverb
				float RDist = 0.0f;

				for (int i = Index; i < n-1; ++i)
				{
					const FVector P_1 = PathPoints[i];
					const FVector P_2 = PathPoints[i + 1];

					const float d = FVector::Dist(P_1, P_2);

					RDist += d;
				}
				NewReverbValue = RadioReverbDist->GetFloatValue(RDist);
				*/
			//}
		}


		
		//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, DistanceFadeCurve->GetFloatValue(dist), DeltaTime, 0.3f);
	}
	else
	{
		//CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, DistanceFadeCurve->GetFloatValue(dist), DeltaTime, 0.3f);
	}
	//CurrentReverbValue = FMath::FInterpConstantTo(CurrentReverbValue, NewReverbValue, DeltaTime, AudioFadeSpeed);
	CurrentAudioVolume = FMath::FInterpConstantTo(CurrentAudioVolume, NewAudioVolume * NewAudioMultiplier, DeltaTime, AudioFadeSpeed);
	SoundLocation->SetWorldLocation(FMath::VInterpConstantTo(SL, NewSoundLocation, DeltaTime, AudioMoveSpeed));
	AC->SetVolumeMultiplier(CurrentAudioVolume);
	//AC->SetFloatParameter(FName("RDist"), CurrentReverbValue);
	//UE_LOG(LogTemp, Warning, TEXT("ReverbValue: %f"), CurrentReverbValue);

	//UE_LOG(LogTemp, Warning, TEXT("AudioVolume: %f"), CurrentAudioVolume);
	//UE_LOG(LogTemp, Warning, TEXT("n: %d"), n);
}

void ARadio::GetNewPath()
{
	const FVector RL = PathStart->GetComponentLocation();
	const FVector PL = Player->GetActorLocation();
	UNavigationPath * Path = NavigationSystem->FindPathToLocationSynchronously(World, PL, RL);
	NavPathPoints = Path->PathPoints;
}

