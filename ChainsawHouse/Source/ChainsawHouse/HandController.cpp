// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Door.h"
#include "XRMotionControllerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Flashlight.h"
#include "Grabbable.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Math/RotationAboutPointMatrix.h"
#include "Bottle.h"
#include "DestructibleComponent.h"
#include "Chainsaw.h"
#include "Drawer.h"
#include "Ball.h"
#include "VRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	SetRootComponent(MotionController);

	GrabFlashlight = CreateDefaultSubobject<USceneComponent>(TEXT("GrabFlashlight"));
	GrabFlashlight->AttachTo(MotionController);

	GrabBall = CreateDefaultSubobject<USceneComponent>(TEXT("GrabBall"));
	GrabBall->AttachTo(MotionController);

	GrabPhone = CreateDefaultSubobject<USceneComponent>(TEXT("GrabPhone"));
	GrabPhone->AttachTo(MotionController);

	GrabKey = CreateDefaultSubobject<USceneComponent>(TEXT("GrabKey"));
	GrabKey->AttachTo(MotionController);

	GrabSaw2 = CreateDefaultSubobject<USceneComponent>(TEXT("GrabSaw2"));
	GrabSaw2->AttachTo(MotionController);

	State = HandControllerState::STATE_IDLE;
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);

	UActorComponent * MeshComponent = GetComponentByClass(USkeletalMeshComponent::StaticClass());
	HandMesh = Cast<USkeletalMeshComponent>(MeshComponent);
	if (HandMesh == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh cast failed in HandController.cpp"));
	}

	HandMeshRelativeTransform = HandMesh->GetRelativeTransform();
	ChainsawOffset = Cast<USceneComponent>(GetComponentsByTag(USceneComponent::StaticClass(), FName("Saw"))[0]);

	BallOffset = HandMesh->GetComponentLocation() - HandMesh->GetSocketLocation(FName("BallSocket"));

	DeltaLocation = MotionController->GetComponentLocation();

	Dog = Cast<ADog>(UGameplayStatics::GetActorOfClass(GetWorld(), ADog::StaticClass()));
	Player = Cast<AVRCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), AVRCharacter::StaticClass()));

	CollisionCapsule = Cast<UPrimitiveComponent>(GetComponentByClass(UCapsuleComponent::StaticClass()));

	/*
	TArray<UActorComponent*> GrabSceneComponents = GetComponentsByTag(USceneComponent::StaticClass(), FName("GrabScene"));

	for (auto i : GrabSceneComponents)
	{
		if (i->ComponentHasTag(FName("Flashlight"))) 
		{
			GrabFlashlight = Cast<USceneComponent>(i);
		}
		if (i->ComponentHasTag(FName("Ball")))
		{
			GrabBall = Cast<USceneComponent>(i);
		}
	}*/

}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaLocation = MotionController->GetComponentLocation() - DeltaLocation;
	HandControllerVelocity = DeltaLocation;

	//UE_LOG(LogTemp, Warning, TEXT("DL: %f, %f, %f"), DeltaLocation.X, DeltaLocation.Y, DeltaLocation.Z);

	//DrawDebugLines(DeltaTime);	///////////////////// DEBUG HELPER

	if (bIsClimbing)
	{
		FVector HandControllerDelta = GetActorLocation() - ClimbingStartLocation;
		GetAttachParentActor()->AddActorWorldOffset(-HandControllerDelta, true);
	}
	else if (bIsUsingDoor)
	{
		CheckDoorDistance();
	}

	DeltaLocation = MotionController->GetComponentLocation();

	if (GrabFlashlight != nullptr)
	{
		//DrawGrabSceneOffset();
	}
}

void AHandController::Grip()
{
	bIsGripping = true;

	if (bCanGrab)
	{
		if (!bIsGrabbing)
		{
			bIsGrabbing = true;
			//AGrabbable * ActorToGrab = Cast<AGrabbable>(GrabActor);
			ActorBeingGrabbed = Cast<AGrabbable>(GrabActor);

			if (ActorBeingGrabbed != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("GA: %s"), *ActorBeingGrabbed->GetName());
				if (!ActorBeingGrabbed->bTwoHanded)
				{
					if (ActorBeingGrabbed == SisterController->ActorBeingGrabbed)
					{
						SisterController->Release();
						UE_LOG(LogTemp, Warning, TEXT("Sister Release: %s"), *ActorBeingGrabbed->GetName());


					}
				}
				
				//	TWO HANDED MECHANICS
				//if (!ActorBeingGrabbed->bBeingHeld)
				//{
					bIsControllingItem = true;
					ActorBeingGrabbed->bBeingHeld = true;

					UPrimitiveComponent * Mesh;
					Mesh = Cast<UPrimitiveComponent>(GrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
					if (Mesh != nullptr)
					{
						FVector AL = GetActorLocation();
						float D1 = FVector::Distance(AL, ActorBeingGrabbed->HandHold1->GetComponentLocation());
						float D2 = 99999.f;
						USceneComponent* HH2 = ActorBeingGrabbed->HandHold2;

						if (HH2 != nullptr)
						{
							D2 = FVector::Distance(AL, ActorBeingGrabbed->HandHold2->GetComponentLocation());
						} 

						if (D1 > D2)
						{
							//HandMesh->AttachToComponent(ActorBeingGrabbed->HandHold2, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							
							// this might be causing the bug where picking up ball drops the flashlight
							if (ActorBeingGrabbed->MotionController2 == SisterController)
							{
								SisterController->Release();
							}

							if (bLeft)
							{
								HandMesh->AttachToComponent(ActorBeingGrabbed->HandL2, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							}
							else
							{
								HandMesh->AttachToComponent(ActorBeingGrabbed->HandR2, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							}

							ActorBeingGrabbed->MotionController2 = this;
							ActorBeingGrabbed->Gripped(2);
							bHandHold2 = true;
							
							// two handed grab scene offset stuff goes here

							if (GrabActor->ActorHasTag(TEXT("Saw")))
							{
								bIsHoldingChainsaw = true;

								GrabSceneOffset = GrabSaw2;
								State = HandControllerState::STATE_CHAINSAW2;

								AChainsaw * C = Cast<AChainsaw>(GrabActor);

								// need to fix this
								if (C != nullptr && C->bDismembering)
								{
									return;
								}
							}

						}
						else
						{
							//HandMesh->AttachToComponent(ActorBeingGrabbed->HandHold1, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							
							if (ActorBeingGrabbed->MotionController1 == SisterController)
							{
								SisterController->Release();
							}

							if (bLeft)
							{
								HandMesh->AttachToComponent(ActorBeingGrabbed->HandL1, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							}
							else
							{
								HandMesh->AttachToComponent(ActorBeingGrabbed->HandR1, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
							}

							ActorBeingGrabbed->MotionController1 = this;
							ActorBeingGrabbed->Gripped(1);
							bHandHold1 = true;
							//need to apply rotation as well

							if (GrabActor->ActorHasTag(TEXT("Saw")))
							{
								bIsHoldingChainsaw = true;

								GrabSceneOffset = GetRootComponent();
								//GrabSceneOffset = GrabSaw1;
								State = HandControllerState::STATE_CHAINSAW1;

								AChainsaw * C = Cast<AChainsaw>(GrabActor);

								// need to fix this
								if (C->bDismembering)
								{
									return;
								}
							}
						}

						if (GrabActor->ActorHasTag(TEXT("Flashlight")))
						{
							State = HandControllerState::STATE_FLASHLIGHT;
							bIsHoldingFlashlight = true;
							GrabSceneOffset = GrabFlashlight;

							UE_LOG(LogTemp, Warning, TEXT("Holding FLashlight"));
							// update animation to holding flashlight pose
							// update location / rotation 
							// EACH GRABBABLE SHOULD HAVE A GRABLOCATION OFFSET AND A GRABROTATION OFFSET FOR EACH HANDHOLD
						}
						else if (GrabActor->ActorHasTag(TEXT("Ball")))
						{
							State = HandControllerState::STATE_BALL;
							GrabSceneOffset = GrabBall;  
							//HandMesh->SetRelativeLocation(BallOffset);
							UpdateAnimation();
							// change dog's look at actor to this ball
							// fetch on release
							//Dog->MoveToPlayer();
							Dog->ChangeLookAtComponent(ActorBeingGrabbed->Mesh);
						}
						else if (GrabActor->ActorHasTag(TEXT("Phone")))
						{
							State = HandControllerState::STATE_PHONE;
							UpdateAnimation();
							GrabSceneOffset = GrabPhone;
						}
						else if (GrabActor->ActorHasTag(TEXT("Key")))
						{
							State = HandControllerState::STATE_KEY;
							GrabSceneOffset = GrabKey;
						}

						/*
						// attach handmesh to chainsaw
						// interp chainsaw to HandController
						

						Mesh->SetSimulatePhysics(false);
						Mesh->SetEnableGravity(false);
						GripSize = ActorBeingGrabbed->ItemGripSize;
						USceneComponent* Root = GrabActor->GetRootComponent();
						bool b = Root->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						
						FVector ChainsawOffset = ActorBeingGrabbed->HandHoldOffset1;

						if (bLeft)
						{
							//ActorBeingGrabbed->NegateYAxisOffset
							ChainsawOffset.Y = -ChainsawOffset.Y;
							ActorBeingGrabbed->SetActorRelativeLocation(ChainsawOffset);
						}
						else
						{
							ActorBeingGrabbed->SetActorRelativeLocation(ChainsawOffset);
						}
						*/
						UpdateAnimation();

						ActorBeingGrabbed->GripTranslate();

					}
				//}
				/*
				else
				{
					HandMesh->AttachToComponent(ActorBeingGrabbed->HandHold2, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					/*
					
					GripSize = ActorBeingGrabbed->ItemGripSize;

					ActorBeingGrabbed->MC1OffsetComponent = ChainsawOffset;
					ActorBeingGrabbed->MotionController1 = this;
					ActorBeingGrabbed->MotionController2 = SisterController;
					ActorBeingGrabbed->bRotateTwoHand = true;
					*/
					// also need to add the offset so the hand lines up perfectly with the mesh
				//}
				


				// I NEED TO FIX THIS CODE
				/*
				if (ActorBeingGrabbed == SisterController->ActorBeingGrabbed)
				{
					SisterController->Release();
				}
				
				// I could make this mesh global
				UPrimitiveComponent * Mesh;
				Mesh = Cast<UPrimitiveComponent>(GrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
				if (Mesh != nullptr)
				{
					FName SocketName = TEXT("");
					if (GrabActor->ActorHasTag(TEXT("Flashlight")))
					{
						bIsHoldingFlashlight = true;
						SocketName = TEXT("GrabSocket");
					}
					else if (GrabActor->ActorHasTag(TEXT("Chainsaw")))
					{
						bIsHoldingChainsaw = true;
						SocketName = TEXT("ChainSocket");
					}
					else if (GrabActor->ActorHasTag(TEXT("Bot")))
					{
						UE_LOG(LogTemp, Warning, TEXT("Grabbing Bot"));
						bIsHoldingBottle = true;
						SocketName = TEXT("ChainSocket");
					}
					else if (GrabActor->ActorHasTag(TEXT("Ball")))
					{
						UE_LOG(LogTemp, Warning, TEXT("Grabbing ball!"));
						bIsHoldingBall = true;
						SocketName = TEXT("ChainSocket");
					}
					
					Mesh->SetSimulatePhysics(false);
					GripSize = ActorBeingGrabbed->ItemGripSize;
					GrabActor->AttachToComponent(HandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
					
					// if this is a left hand then rotate the item being picked up 180 degrees around the socket's up vector
					//	THIS IF() MIGHT NOT BE NECCASARY
					if (bLeft)
					{
						USceneComponent * HandMeshCasted;
						HandMeshCasted = Cast<USceneComponent>(HandMesh);

						if (HandMeshCasted != nullptr)
						{
							
							FQuat Q = GrabActor->GetActorQuat();
							FVector FV = GrabActor->GetActorForwardVector();
							FVector UV = GrabActor->GetActorUpVector();

							FQuat NewRot = FQuat(UV, PI);

							GrabActor->SetActorRelativeRotation(NewRot);

						}
					}

					//UE_LOG(LogTemp, Warning, TEXT("%s"), *ActorToGrab->GetName());
				}
				*/
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ActorBeingGrabbed cast failed!"));
			}
			return;
		}
	}
	else if (bCanClimb)
	{
		if (!bIsClimbing)
		{
			bIsClimbing = true;
			ClimbingStartLocation = GetActorLocation();
			GripSize = 80.f;

			SisterController->bIsClimbing = false; // Hopfully this doesn't cause any issues. I'm puttting this here so PortalLadderRoom knows which HandController's ClimbingStartLocation to use.

			AVRCharacter * VRChar = Cast<AVRCharacter>(GetAttachParentActor());

			if (VRChar->bClimbing != true)
			{
				// update capsule height and keep VRRoot in same location relative to starting capsule height
				VRChar->GetCharacterMovement()->GravityScale = 0;
				VRChar->bClimbing = true;
			}

			VRChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			VRChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);

			return;
		}
	}
	else if (bCanUseDoor)
	{
		if (!bIsUsingDoor)
		{
			bIsUsingDoor = true;
			Player->bIsUsingDoor = true;
			
			//OverlappingKnob = OverlappingDoor->GetRootComponent()->GetChildComponent(2);	/////////////////////////////////
			//UE_LOG(LogTemp, Warning, TEXT("O: %s"), OverlappingKnob->GetName());

			ADoor* CurrentDoor = Cast<ADoor>(OverlappingDoor);

			if (CurrentDoor != nullptr)
			{
				CurrentDoor->PassController(this);
				//GripSize = 80.f;

				
				AttachHandMeshToDoor(CurrentDoor);

				State = HandControllerState::STATE_DOOR;

				if (CurrentDoor->ActorHasTag(FName("MainDoor")))
				{
					State = HandControllerState::STATE_DOORMAIN;
				}

				UpdateAnimation();

				KnobBeingGrabbed = CurrentDoor->Doorknob;

				// Attach this hand controller's skeletal mesh to the doorknob.

				// Initiate knob turn mechanics if the door is currently closed all the way.

				// DEBUG UNLOCK DOOR
				//CurrentDoor->bLocked = false;

				return;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("CurrentDoor cast failed!!!"));
			}
			//UsingDoorLocation = GetActorLocation();
		}
	}
	else if (bCanUseDrawer)
	{
		if (!bIsUsingDrawer)
		{
			bIsUsingDrawer = true;

			ADrawer * CurrentDrawer = Cast<ADrawer>(GrabActor);
			
			if (CurrentDrawer != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("BIG DRAWER!!!!!"));
				CurrentDrawer->GrabDrawer(this);
				HandMesh->AttachToComponent(GrabActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				// need to change hand animation and add an offset here to grab the handle
			}
		}
	}
	GripSize = GripSizeMax;
}

void AHandController::Release()
{
	bIsGripping = false;

	//climb
	if (bIsClimbing)
	{
		bIsClimbing = false;
		
		// Have the player start updating their capsule height again
		if (!SisterController->bIsClimbing)
		{
			AVRCharacter * VRChar = Cast<AVRCharacter>(GetAttachParentActor());
			VRChar->bClimbing = false;
			VRChar->GetCharacterMovement()->GravityScale = 1.0f;
			VRChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			VRChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		}
		else
		{

		}

	}
	// door
	if (bIsUsingDoor)
	{
		bIsUsingDoor = false;
		
		if (!SisterController->bIsUsingDoor)
		{
			Player->bIsUsingDoor = false;
		}

		ADoor* CurrentDoor = Cast<ADoor>(OverlappingDoor);

		if (CurrentDoor != nullptr && !CurrentDoor->bLocked)
		{
			CurrentDoor->SetIsBeingUsed(false);
		}

		// remove collision on HC for 0.5 seconds.
		RemoveHCCol();

		DetachHandMeshAndReattachToHC();

		KnobBeingGrabbed = nullptr;

		// Attach HandMesh to this hand controller

		//free(OverlappingDoor);
	}
	else if (bIsUsingDrawer)
	{
		bIsUsingDrawer = false;
		ADrawer * CurrentDrawer = Cast<ADrawer>(GrabActor);
		if (CurrentDrawer != nullptr)
		{
			CurrentDrawer->bBeingGrabbed = false;
		}
		DetachHandMeshAndReattachToHC();
	}

	if (bIsGrabbing)
	{
		bIsGrabbing = false;

		if (GrabActor != nullptr)
		{
			if (GrabActor->ActorHasTag(TEXT("Flashlight")))
			{
				bIsHoldingFlashlight = false;
			}
			else if (GrabActor->ActorHasTag(TEXT("Saw")))
			{
				AChainsaw * C = Cast<AChainsaw>(GrabActor);

				if (C != nullptr)
				{
					if (C->bDismembering)
					{
						bIsGripping = true;
						bIsGrabbing = true;
						return;
					}
					else
					{
						bIsHoldingChainsaw = false;

						if (bRevvingChainsaw)
						{
							bRevvingChainsaw = false;

							C->ReleaseTrigger();
						}
					}
				}
			}
			else if (GrabActor->ActorHasTag(TEXT("Bot")))
			{
				bIsHoldingBottle = false;
			}
			else if (GrabActor->ActorHasTag(TEXT("Ball")))
			{
				bIsHoldingBall = false;

				
				HandControllerState::STATE_IDLE;

				if (Dog->bWantsToFetch)
				{
					Dog->bFetchWhenReady = true;
					//Dog->FetchBall();
				}
				//tell dog to fetch
			}


			// two handed mechanics
			if (ActorBeingGrabbed != nullptr)
			{
				DetachHandMeshAndReattachToHC();

				if (bIsControllingItem)
				{
					ActorBeingGrabbed->bBeingHeld = false;
					bIsControllingItem = false;
				}
				if (bHandHold1)
				{
					ActorBeingGrabbed->Released(1);
					bHandHold1 = false;
				}
				else if (bHandHold2)
				{
					ActorBeingGrabbed->Released(2);
					bHandHold2 = false;
				}
				/*
				if (bIsControllingItem)
				{
					ActorBeingGrabbed->bBeingHeld = false;
					bIsControllingItem = false;

					UPrimitiveComponent * Mesh = nullptr;
					Mesh = Cast<UPrimitiveComponent>(GrabActor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
					if (Mesh != nullptr)
					{
						USceneComponent* Root = GrabActor->GetRootComponent();
						Root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

						Mesh->SetSimulatePhysics(true);
						Mesh->SetEnableGravity(true);


						GrabActor = nullptr;
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Not Controlling item"));
					HandMesh->DetachFromParent();
					HandMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
					HandMesh->SetRelativeTransform(HandMeshRelativeTransform);
					ActorBeingGrabbed->bRotateTwoHand = false;
					// stop attaching handmesh to handhold
				}
				*/	
			}
		}
		
	}

	bIsUsingDoor = false;	// might fix bug where player cant interact with knob until they move away and back again...?

	if (GripSize != GripSizeDefault)
	{
		GripSize = GripSizeDefault;
	}

	State = HandControllerState::STATE_IDLE;
	UpdateAnimation();
}

void AHandController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	//UE_LOG(LogTemp, Warning, TEXT("BeginOverlap: %s"), *OtherActor->GetName());
	if (!bIsGripping)
	{
		// Set bNewCanClimb and bNewCanUseDoor
		CanInteract();

		// Door mechanics
		if (!bIsUsingDoor && !bCanUseDoor && bNewCanUseDoor)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Can use door!"));
			GripSize = GripSizeCanGrab;

			//PlayCanGrabHapticEffect();
		}
		bCanUseDoor = bNewCanUseDoor;

		// Grab mechanics
		if (!bCanGrab && bNewCanGrab)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Can Grab!"));
			GripSize = GripSizeCanGrab;

			//PlayCanGrabHapticEffect();
		}
		bCanGrab = bNewCanGrab;
		// Climb mechanics
		if (!bCanClimb && bNewCanClimb)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Can Climb!"));
			GripSize = GripSizeCanGrab;

			//PlayCanGrabHapticEffect();
		}
		bCanClimb = bNewCanClimb;

		if (!bIsUsingDrawer && !bCanUseDrawer && bNewCanUseDrawer)
		{
			GripSize = GripSizeCanGrab;

			//PlayCanGrabHapticEffect();
		}
	}
}

void AHandController::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	//UE_LOG(LogTemp, Warning, TEXT("EndOverlap: %s"), *OtherActor->GetName());
	if (!bIsGripping)
	{
		//bCanClimb = CanClimb();
		CanInteract();
		bCanClimb = bNewCanClimb;
		bCanUseDoor = bNewCanUseDoor;
		bCanGrab = bNewCanGrab;
		bCanUseDrawer = bNewCanUseDrawer;

		if (GripSize == GripSizeCanGrab)
		{
			GripSize = GripSizeDefault;
		}

		//UE_LOG(LogTemp, Warning, TEXT("End Overlap: Can Grab = %d"), bCanGrab);
	}
}

void AHandController::CanInteract()
{
	//if (State == HandControllerState::STATE_IDLE)
	//{
		TArray<AActor*> OverlappingActors;
		TArray<UPrimitiveComponent*> OverlappingComponents;
		//GetOverlappingActors(OverlappingActors);
		//GetOverlappingComponents(OverlappingComponents);
		CollisionCapsule->GetOverlappingActors(OverlappingActors);
		CollisionCapsule->GetOverlappingComponents(OverlappingComponents);

		for (UPrimitiveComponent* OC : OverlappingComponents)
		{
			if (OC->ComponentHasTag(TEXT("Knob")))
			{
				ADoor *OD = Cast<ADoor>(OC->GetOwner());
				if (OD->bEnabled)
				{
					bNewCanUseDoor = true;
					if (!bIsGripping)
					{
						State = HandControllerState::STATE_CANGRAB;
					}
					//PlayCanGrabHapticEffect();
					OverlappingKnob = OC;
					OverlappingDoor = OD;
					return;
				}
			}
			else if (OC->ComponentHasTag(TEXT("Drawer")))
			{
				bNewCanUseDrawer = true;
				State = HandControllerState::STATE_CANGRAB;
				//PlayCanGrabHapticEffect();
				GrabActor = OC->GetAttachmentRootActor();
				return;
			}
		}

		for (AActor* OverlappingActor : OverlappingActors)
		{
			if (OverlappingActor->ActorHasTag(TEXT("Grab")))
			{
				//UE_LOG(LogTemp, Warning, TEXT("overlap %s"), *OverlappingActor->GetName());

				if (OverlappingActor->IsA(ABall::StaticClass()))
				{
					// cant interact if the ball is being fetched
					ABall *Ball = Cast<ABall>(OverlappingActor);
					if (Ball != nullptr)
					{
						if (Ball->bBeingFetched)
						{
							bNewCanGrab = false;
							return;
						}
					}
				}
				else if (OverlappingActor->IsA(AFlashlight::StaticClass()))
				{
					
				}

				//if(Cast<AGrabbable>(OverlappingActor)->ControllingHandController)

				bNewCanGrab = true;

				if (!bIsGripping)
				{
					State = HandControllerState::STATE_CANGRAB;
				}
				

				//PlayCanGrabHapticEffect();
				GrabActor = OverlappingActor;
				return;
			}
			else if (OverlappingActor->ActorHasTag(TEXT("Climbable")))
			{
				bNewCanClimb = true;
				if (!bIsGripping)
				{
					State = HandControllerState::STATE_CANGRAB;
				}
				//PlayCanGrabHapticEffect();
				return;
			}
			/*
			else if (OverlappingActor->ActorHasTag(TEXT("Door")))
			{
				bNewCanUseDoor = true;
				OverlappingDoor = OverlappingActor;
				return;
			}
			*/
		}

		State = HandControllerState::STATE_IDLE;
		GrabActor = nullptr;
		bNewCanClimb = false;
		bNewCanUseDoor = false;
		bNewCanGrab = false;
		bNewCanUseDrawer = false;
	//}
}

void AHandController::CheckDoorDistance()
{
	float d = FVector::Dist(GetActorLocation(), KnobBeingGrabbed->GetComponentLocation());

	if (d > 110.f)
	{
		Release();
	}
}

void AHandController::DrawDebugLines(float DeltaTime)
{
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorUpVector() * 20.f, FColor::Red, false, 2 * DeltaTime);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorRightVector() * 20.f, FColor::Blue, false, 2 * DeltaTime);
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 20.f, FColor::Green, false, 2 * DeltaTime);
	FVector RotVec = GetActorUpVector();
	RotVec = RotVec.RotateAngleAxis(45.f, GetActorRightVector());

	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + RotVec * 20.f, FColor::Magenta, false, 2 * DeltaTime);
}

void AHandController::PressFlashlightButton()
{
	if (bIsHoldingFlashlight)
	{
		ButtonPress = 50.f;
	}
}

void AHandController::ReleaseFlashlightButton()
{
	if (bIsHoldingFlashlight)
	{
		ButtonPress = 35.f;
	}
}

void AHandController::SetSisterController(AHandController * Sister)
{
	SisterController = Sister;
}

void AHandController::PrintSocketOffsets(float DeltaTime)
{
	USceneComponent * HandMeshCasted;
	HandMeshCasted = Cast<USceneComponent>(HandMesh);

	if (HandMeshCasted != nullptr)
	{
		if (GrabActor != nullptr)
		{
			FName SocketName = TEXT("");
			if (GrabActor->ActorHasTag(TEXT("Flashlight")))
			{
				bIsHoldingFlashlight = true;
				SocketName = TEXT("GrabSocket");
			}
			else if (GrabActor->ActorHasTag(TEXT("Chainsaw")))
			{
				bIsHoldingChainsaw = true;
				SocketName = TEXT("ChainSocket");
			}
			FRotator SocketRotation = HandMeshCasted->GetSocketRotation(SocketName);
			FVector SocketLocation = HandMeshCasted->GetSocketLocation(SocketName);
			FMatrix SockRotPonMat = FRotationAboutPointMatrix::Make(SocketRotation, SocketLocation);
			FVector SFV = SockRotPonMat.GetScaledAxis(EAxis::X);
			FVector SRV = SockRotPonMat.GetScaledAxis(EAxis::Y);
			FVector SUV = SockRotPonMat.GetScaledAxis(EAxis::Z);

			FQuat SQ = HandMeshCasted->GetSocketQuaternion(SocketName);
			FQuat SI = FQuat(SockRotPonMat.Inverse());
			
			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SQ.GetForwardVector() * 15.f, FColor::Red, false, 2 * DeltaTime);
			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SQ.GetRightVector() * 15.f, FColor::Green, false, 2 * DeltaTime);
			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SQ.GetUpVector() * 15.f, FColor::Blue, false, 2 * DeltaTime);
			DrawDebugPoint(GetWorld(), GrabActor->GetActorLocation(), 10.f, FColor::White, false, 2 * DeltaTime);

			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SI.GetForwardVector() * 7.f, FColor::Magenta, false, 2 * DeltaTime);
			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SI.GetRightVector() * 7.f, FColor::Emerald, false, 2 * DeltaTime);
			DrawDebugLine(GetWorld(), SocketLocation, SocketLocation + SI.GetUpVector() * 7.f, FColor::Cyan, false, 2 * DeltaTime);
		}
		
	}
}

void AHandController::DetachHandMeshAndReattachToHC()
{
	HandMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	HandMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	HandMesh->SetRelativeTransform(HandMeshRelativeTransform);
}

void AHandController::AttachHandMeshToDoor(ADoor* TheDoor)
{
	const USceneComponent *DoorHinge = TheDoor->DoorHinge;
	const FVector AL = GetActorLocation();
	const FVector HL = DoorHinge->GetComponentLocation();
	const FVector HFV = DoorHinge->GetForwardVector();
	const FVector Dir = (AL - HL).GetSafeNormal();
	const float Dot = FVector::DotProduct(HFV, Dir);

	const bool bFlipped = TheDoor->bFlipHandLocations;

	if (Dot > 0)
	{
		if (bLeft)
		{
			if (!bFlipped)
			{
				HandMesh->AttachToComponent(TheDoor->GetHandLFront(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				HandMesh->AttachToComponent(TheDoor->GetHandLBack(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			
		}
		else
		{
			if (!bFlipped)
			{
				HandMesh->AttachToComponent(TheDoor->GetHandRFront(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				HandMesh->AttachToComponent(TheDoor->GetHandRBack(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}
	else
	{
		if (bLeft)
		{
			if (!bFlipped)
			{
				HandMesh->AttachToComponent(TheDoor->GetHandLBack(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				HandMesh->AttachToComponent(TheDoor->GetHandLFront(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
		else
		{
			if (!bFlipped)
			{
				HandMesh->AttachToComponent(TheDoor->GetHandRBack(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				HandMesh->AttachToComponent(TheDoor->GetHandRFront(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}
	}

	/*
	never forget the first iteration lol
	HandMesh->AttachToComponent(OverlappingKnob, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if (bLeft)
	{
		HandMesh->AddLocalOffset(FVector(-2.795609f,16.379856f,0.054878f));
		HandMesh->AddLocalRotation(FRotator(38.562122f, -8.343560f, 3.542102f));
	}
	else
	{
		HandMesh->AddLocalOffset(HandMeshDoorOffset);
		HandMesh->AddLocalRotation(FRotator(38.562122f, -167.167526f, 8.788214f));
	}
	*/
}

void AHandController::PlayCanGrabHapticEffect()
{
	APawn* Pawn = Cast<APawn>(GetAttachParentActor());
	if (Pawn != nullptr)
	{
		APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());

		if (Controller != nullptr)
		{
			Controller->PlayHapticEffect(HapticEffect, MotionController->GetTrackingSource());
		}
	}
}

void AHandController::DrawGrabSceneOffset()
{
	const FVector FV = GrabFlashlight->GetForwardVector() * 15.f;
	const FVector RV = GrabFlashlight->GetRightVector() * 15.f;
	const FVector UV = GrabFlashlight->GetUpVector() * 15.f;
	const FVector L = GrabFlashlight->GetComponentLocation();
	
	DrawDebugLine(GetWorld(), L, L + FV, FColor::Red, false, GetWorld()->DeltaTimeSeconds * 1.1f);
	DrawDebugLine(GetWorld(), L, L + RV, FColor::Green, false, GetWorld()->DeltaTimeSeconds * 1.1f);
	DrawDebugLine(GetWorld(), L, L + UV, FColor::Blue, false, GetWorld()->DeltaTimeSeconds * 1.1f);
}