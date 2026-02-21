// Fill out your copyright notice in the Description page of Project Settings.


#include "Character1.h"
#include "Animation/AnimInstance.h"
#include "CharacterAI.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/GameplayStatics.h>


ACharacter1::ACharacter1()
{
    PrimaryActorTick.bCanEverTick = true;

    // === CRITICAL SETTINGS FOR AI-FACING ===
    bUseControllerRotationYaw = true;                  // Allow controller rotation to affect yaw
    GetCharacterMovement()->bOrientRotationToMovement = false;  // Disable auto-rotation from movement
}

// Called when the game starts or when spawned
void ACharacter1::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> FoundAI;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterAI::StaticClass(), FoundAI);

	if (FoundAI.Num() > 0)
	{
		TargetAI = Cast<ACharacterAI>(FoundAI[0]);
	}

}


// Called every frame
void ACharacter1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get the animation instance
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	bool bIsInAction = AnimInstance &&
		(AnimInstance->Montage_IsPlaying(PunchMontage) || AnimInstance->Montage_IsPlaying(BlockMontage));

	// Restore normal speed if not attacking/blocking
	if (!bIsInAction)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;

		// Rotate toward AI if it exists
		if (TargetAI && GetController())
		{
			FVector Direction = TargetAI->GetActorLocation() - GetActorLocation();
			Direction.Z = 0.f;

			if (!Direction.IsNearlyZero())
			{
				Direction.Normalize();
				FRotator TargetRotation = Direction.Rotation();

				// Smoothly interpolate controller rotation
				FRotator NewRotation = FMath::RInterpTo(GetControlRotation(), TargetRotation, DeltaTime, RotationSpeed);

				// Keep only yaw
				NewRotation.Pitch = 0.f;
				NewRotation.Roll = 0.f;

				GetController()->SetControlRotation(NewRotation);
			}
		}
	}
}


// Called to bind functionality to input
void ACharacter1::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter1::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacter1::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACharacter1::MoveRight);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACharacter1::Attack);
	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &ACharacter1::Block);
}

void ACharacter1::MoveForward(float Value)
{
	FVector ForwardDirection = GetActorForwardVector();
	AddMovementInput(ForwardDirection, Value);
}

void ACharacter1::MoveRight(float Value)
{
	FVector RightDirection = GetActorRightVector();
	AddMovementInput(RightDirection, Value);
}

void ACharacter1::Attack()
{
	if(PunchMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (!AnimInstance->Montage_IsPlaying(PunchMontage))
		{
			AnimInstance->Montage_Play(PunchMontage);
		}
		GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	}
}

void ACharacter1::Block()
{
	if (BlockMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (!AnimInstance->Montage_IsPlaying(BlockMontage))
		{
			AnimInstance->Montage_Play(BlockMontage);
		}
		GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	}
}


