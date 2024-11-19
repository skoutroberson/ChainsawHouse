// Fill out your copyright notice in the Description page of Project Settings.


#include "DogController.h"
#include "Dog.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationData.h"
#include "DrawDebugHelpers.h"

void ADogController::BeginPlay()
{
	Super::BeginPlay();

	Dog = Cast<ADog>(UGameplayStatics::GetActorOfClass(GetWorld(), ADog::StaticClass()));

	World = GetWorld();

	
}

void ADogController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	switch(Result.Code)
	{
	case EPathFollowingResult::Success:
		UE_LOG(LogTemp, Warning, TEXT("DOG PATH SUCCEEDED"));

		if (Dog->State == DogState::STATE_FETCHING)
		{
			if (bMovingToUnderBall)
			{
				bMovingToUnderBall = false;

				//MoveToActor(Dog->Ball, 1.0f);
				Dog->DelayedMoveToBall();
			}
			else
			{
				bFailFlag = true;
				++FailCount;
				// maybe this will fix?
				Dog->DelayedMoveToBall();
			}
		}
			break;
	case EPathFollowingResult::Aborted:
		if (Result.IsSuccess())
		{
			UE_LOG(LogTemp, Warning, TEXT("DOG PATH ABORTED AND SUCCEEDED"));
		}
		else if (Result.IsFailure())
		{
			UE_LOG(LogTemp, Warning, TEXT("DOG PATH ABORTED AND FAILED"));
			if (Dog->State == DogState::STATE_FETCHING)
			{
				// play sad sound
				if (!Dog->bRanTowardsHouse && Dog->bWantsToFetch)
				{
					//MoveToActor(Dog->Ball, 1.0f);
					//Dog->State = DogState::STATE_SITTINGDOWN;
					Dog->DelayedMoveToBall();
				}
			}
			else
			{
				// play sad sound
				//Dog->DropBall();
			}
		}
		else if (Result.IsInterrupted())
		{
			UE_LOG(LogTemp, Warning, TEXT("DOG PATH ABORTED BECAUSE OF INTERRUPTION"));
		}
		break;
	case EPathFollowingResult::Blocked:
		UE_LOG(LogTemp, Warning, TEXT("DOG BLOCKED YO"));
		break;
	case EPathFollowingResult::Invalid:
		UE_LOG(LogTemp, Warning, TEXT("DOG MOVE REQUEST INVALID"));
		if (Dog->State == DogState::STATE_FETCHING)
		{
			// play sad sound

			// check if this fails like 10 times in a row, if so then just have the dog pick up the ball with super powers fuck it!

			// ball is too high? run to location under ball, on a timer, check if ball is on ground, if it is, then move to the ball

			UE_LOG(LogTemp, Warning, TEXT("Ball too high?"));

			// not checking if dog is null might crash but i dont think it will ;)
			const FVector BP = Dog->Ball->GetActorLocation();
			FVector OL = FVector::ZeroVector;

			FVector QueryExtent = FVector(300.f, 300.f, 300.f);

			FNavAgentProperties NavAgentProps = FNavAgentProperties(20.f, 20.f);

			FNavLocation OutLocation;

			// loop to find closet point on nav mesh to the ball !!!DO THIS ON A TIMER NOT EVERY TICK!!!
			for (int i = 1; i < 5; ++i)
			{
				bool b = World->GetNavigationSystem()->GetMainNavData()->ProjectPoint(BP, OutLocation, QueryExtent * i, nullptr, Dog);
				if (b)
				{
					//DrawDebugPoint(World, OutLocation.Location, 10.f, FColor::Red, true);
					// move to location, and then move to the ball again

					bMovingToUnderBall = true;

					MoveToLocation(OutLocation.Location, 300.f, false, true, true, true);

					UE_LOG(LogTemp, Warning, TEXT("Ball Check Succeed!"));
					break;
				}
				UE_LOG(LogTemp, Warning, TEXT("Ball Check Failed!"));
			}

			// do this after like 100 fails in a row

			Dog->DelayedMoveToBall();
			bFailFlag = true;
			++FailCount;

			//Dog->State = DogState::STATE_SITTINGDOWN;
		}
		else
		{
			// play sad sound

			if (Dog->GetCharacterMovement()->MaxWalkSpeed == 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("POOPY DOOPY HEHE"));
			}
			Dog->DropBall();
		}
		break;
	case EPathFollowingResult::OffPath:
		UE_LOG(LogTemp, Warning, TEXT("DOG IS NOT ON NAV MESH :("));
		break;
	}

	if (Dog->State == DogState::STATE_FOLLOW)
	{
		// delay and follow again
	}

	if (!bFailFlag)
	{
		FailCount = 0;
	}
	else
	{
		if (FailCount > 30)
		{
			FailCount = 0;

			UPrimitiveComponent *BallMesh = Dog->Ball->Mesh;
			if (BallMesh != nullptr)
			{
				BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				BallMesh->SetPhysicsAngularVelocity(FVector::ZeroVector);
				BallMesh->SetWorldLocation(Dog->GetActorLocation() + Dog->GetActorForwardVector() * 10.f);
			}
		}
	}
}