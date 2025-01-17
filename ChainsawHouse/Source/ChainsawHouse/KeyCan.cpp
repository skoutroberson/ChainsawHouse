// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyCan.h"

// Sets default values
AKeyCan::AKeyCan()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AKeyCan::BeginPlay()
{
	Super::BeginPlay();
	
	StartZ = GetActorLocation().Z;

	SpawnLocation = GetActorLocation();
	SpawnRotation = GetActorRotation();
}

// Called every frame
void AKeyCan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCheckZ)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CheckZ"));
		CheckZ();
	}
}

void AKeyCan::AttachKey(ADoorKey * Key)
{
	if (Key != nullptr)
	{
		bHasKey = true;
		AttachedKey = Key;
		const FAttachmentTransformRules ATR = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		Key->AttachToActor(this, ATR);
		Key->SetActorRelativeLocation(FVector(0.0f, 0.0f, 1.0f));
		Key->SetActorRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));
		//Key->AddActorWorldOffset(FVector(0, 0, 4.f)); old shit way of doing it
		UE_LOG(LogTemp, Warning, TEXT("AttachKey"));
		// set relative location/rotation so it is laying in the can
	}
}

void AKeyCan::DetachKey()
{
	AttachedKey->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	UPrimitiveComponent * PC = Cast<UPrimitiveComponent>(AttachedKey->GetRootComponent());

	AttachedKey->GetRootComponent()->SetVisibility(true);
	PC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PC->SetSimulatePhysics(true);
	PC->SetEnableGravity(true);
	PC->AddImpulse(GetActorUpVector() * 4.f);
	float r = FMath::RandRange(0.0f, 1.0f);
	PC->AddAngularImpulse(FVector(r,r,r));
	bHasKey = false;
	AttachedKey = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("DetachKey"));
}

void AKeyCan::CheckZ()
{
	const float CurrentZ = GetActorLocation().Z;

	if(CurrentZ <= StartZ - EndZDistance)
	{
		// detach key
		bCheckZ = false;
		SetActorTickEnabled(false);
		bHasKey = false;
		DetachKey();
		UE_LOG(LogTemp, Warning, TEXT("Key Detached"));
	}
}

