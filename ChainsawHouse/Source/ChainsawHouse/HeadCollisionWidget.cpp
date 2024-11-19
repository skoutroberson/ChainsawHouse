// Fill out your copyright notice in the Description page of Project Settings.


#include "HeadCollisionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "VRCharacter.h"
#include "Camera/CameraComponent.h"

// Sets default values
AHeadCollisionWidget::AHeadCollisionWidget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// this doesn't work with UWidgetComponent in UE 4.26
	//HCWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HCWidget"));
	//HCWidget->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AHeadCollisionWidget::BeginPlay()
{
	Super::BeginPlay();

	PlayerCamera = Cast<USceneComponent>(UGameplayStatics::GetActorOfClass(GetWorld(), AVRCharacter::StaticClass())->
		GetComponentByClass(UCameraComponent::StaticClass()));
}

// Called every frame
void AHeadCollisionWidget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckLocationAndRotation(DeltaTime);
}

void AHeadCollisionWidget::CheckLocationAndRotation(float DeltaTime)
{
	const FVector AL = GetActorLocation();
	const FQuat AQ = GetActorQuat();
	const FVector AFV = GetActorForwardVector();

	const FVector PL = PlayerCamera->GetComponentLocation();
	const FQuat PQ = PlayerCamera->GetComponentQuat();
	const FVector PFV = PlayerCamera->GetForwardVector();

	const float Dist = FVector::Dist(AL, PL);

	if (Dist > CorrectDistance)
	{
		bCorrectingLocation = true;
	}
	else if (Dist <= 0.1f)
	{
		bCorrectingLocation = false;
	}

	const float Dot = FVector::DotProduct(AFV, PFV);

	if (Dot < CorrectDot)
	{
		bCorrectingRotation = true;
	}
	else if (Dot >= 0.99f)
	{
		bCorrectingRotation = false;
	}

	if (bCorrectingLocation)
	{
		const FVector NewLocation = FMath::VInterpTo(AL, PL, DeltaTime, MoveSpeed);
		SetActorLocation(NewLocation);
	}
	if (bCorrectingRotation)
	{
		const FQuat NewRotation = FMath::QInterpTo(AQ, PQ, DeltaTime, RotateSpeed);
		SetActorRotation(NewRotation);
	}
}

