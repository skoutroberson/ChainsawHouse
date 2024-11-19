// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeadCollisionWidget.generated.h"

UCLASS()
class CHAINSAWHOUSE_API AHeadCollisionWidget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHeadCollisionWidget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent *Root = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent *PlayerCamera = nullptr;

	/**
	* UE 4.26 is bugged and it gives a LINKR error if I try to set this up in C++ with CreateDefaultSubobject in the constructor
	* I have to get this variable from the blueprint in BeginPlay()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetComponent *HCWidgetCPP = nullptr;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FName UIText = "Move head down X cm to proceed";
	*/

	// if distance with player camera is greater than this then we will correct it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorrectDistance = 10.f;

	// if dot product with camera foward vector is less than this, then we will correct it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorrectDot = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCorrectingLocation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCorrectingRotation = false;

	void CheckLocationAndRotation(float DeltaTime);
};
