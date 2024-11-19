// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "VRCharacter.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StereoLayerComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APauseMenu::APauseMenu()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APauseMenu::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);

	PlayerCamera = Cast<USceneComponent>(UGameplayStatics::GetActorOfClass(GetWorld(), AVRCharacter::StaticClass())->GetComponentByClass(UCameraComponent::StaticClass()));
	
	StereoLayerComp = Cast<UStereoLayerComponent>(GetComponentByClass(UStereoLayerComponent::StaticClass()));
	//WidgetComp = Cast<UWidgetComponent>(GetComponentByClass(UWidgetComponent::StaticClass()));
	WidgetComp = Cast<USceneComponent>(GetComponentsByTag(USceneComponent::StaticClass(), FName("Widge"))[0]);
}

// Called every frame
void APauseMenu::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckLocationAndRotation();

	//if (bUpdateRotationAndLocation)
	//{
	UpdateLocationAndRotation(DeltaTime);
	//}
}

void APauseMenu::CheckLocationAndRotation()
{
}

void APauseMenu::UpdateLocationAndRotation(float DeltaTime)
{
	const FVector CL = PlayerCamera->GetComponentLocation();
	const FVector AL = GetActorLocation();
	//const FVector WL = WidgetComp->GetComponentLocation();

	FRotator AR = GetActorRotation();
	FRotator CR = PlayerCamera->GetComponentRotation();
	//const float CameraRoll = CR.Roll;
	//AR.Roll = 0.0f;
	//CR.Roll = 0.0f;

	const FQuat CQ = PlayerCamera->GetComponentQuat();

	//SetActorLocation(CL);

	SetActorLocationAndRotation(CL, CQ);
/*
	SetActorLocationAndRotation(FMath::VInterpTo(AL, CL, DeltaTime, 8.f), FMath::RInterpTo(AR, CR, DeltaTime, 4.f));

	const FVector CFV = PlayerCamera->GetForwardVector();
	const FVector CRV = PlayerCamera->GetRightVector();
	const FVector CUV = PlayerCamera->GetUpVector();

	const FVector Disp = WL - CL;
	const FVector Dir = Disp.GetSafeNormal();
	const float Size = Disp.Size();

	float FDot = FVector::DotProduct(Dir, CFV) * Size;
	float RDot = FVector::DotProduct(Dir, CRV) * Size;
	float UDot = FVector::DotProduct(Dir, CUV) * Size;

	const FVector RelativeV = FVector(FDot, RDot, UDot);

	const FVector RV = StereoLayerComp->GetRelativeLocation();
	const FRotator SR = StereoLayerComp->GetRelativeRotation();

	const FTransform CT = PlayerCamera->GetComponentTransform();

	const FRotator WR = WidgetComp->GetComponentRotation();

	//StereoLayerComp->SetRelativeLocation(FMath::VInterpTo(RV, RelativeV, DeltaTime, 8.f));

	//StereoLayerComp->SetRelativeLocationAndRotation(FMath::VInterpTo(RV, RelativeV, DeltaTime, 5.f), FRotator(0.0f, 0.0f, -CameraRoll));

	const FVector NewLocation = UKismetMathLibrary::InverseTransformLocation(CT, WL);
	const FRotator NewRotation = UKismetMathLibrary::InverseTransformRotation(CT, WR);

	StereoLayerComp->SetRelativeLocationAndRotation(FMath::VInterpTo(RV, NewLocation, DeltaTime, 5.f), 
		FRotator(0.0f, 0.0f, -CameraRoll));
	
	

	FVector CRV = PlayerCamera->GetRightVector();
	FVector AFV = GetActorForwardVector();
	FVector CFV = PlayerCamera->GetForwardVector();

	const float Dot = FVector::DotProduct(CRV, AFV);

	const FVector GL = FVector(600.f, FMath::Sin(Dot) * StereoLayerOffsetMultiplier, 0.0f);

	const FVector RL = StereoLayer->GetRelativeLocation();
	
	//StereoLayer->SetRelativeLocation(FMath::VInterpTo(RL, GL, DeltaTime, 6.f));

	FRotator SR = StereoLayer->GetComponentRotation();

	StereoLayer->SetRelativeLocationAndRotation(FMath::VInterpTo(RL, GL, DeltaTime, 5.f), FRotator(0.0f, 0.0f, -CameraRoll));
	
	//SetActorLocationAndRotation(CL, CQ);

	//SetActorLocationAndRotation(FMath::VInterpTo(AL, CL, DeltaTime, 5.0f), FMath::RInterpTo(AR, GR, DeltaTime, 5.0f));

	//StereoLayer->SetRelativeRotation(FRotator(0.0f, 0.0f, -CR.Roll));
	*/
}

void APauseMenu::PauseGame()
{
	const FVector CL = PlayerCamera->GetComponentLocation();
	const FQuat CQ = PlayerCamera->GetComponentQuat();

	SetActorLocationAndRotation(CL, CQ);

	SetActorTickEnabled(true);
}

void APauseMenu::UnPauseGame()
{
	bUpdateRotationAndLocation = false;
	SetActorTickEnabled(false);
}

