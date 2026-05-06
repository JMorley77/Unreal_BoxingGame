// Fill out your copyright notice in the Description page of Project Settings.


#include "Character1.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	bool bIsInAction = AnimInstance &&
		(AnimInstance->Montage_IsPlaying(PunchMontage) || AnimInstance->Montage_IsPlaying(BlockMontage));

	// Speed control (this can stay conditional)
	if (!bIsInAction)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}

	//  ALWAYS rotate toward target
	if (TargetAI && GetController())
	{
		FVector Direction = TargetAI->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f;

		if (!Direction.IsNearlyZero())
		{
			Direction.Normalize();
			FRotator TargetRotation = Direction.Rotation();

			FRotator NewRotation = FMath::RInterpTo(
				GetControlRotation(),
				TargetRotation,
				DeltaTime,
				RotationSpeed
			);

			NewRotation.Pitch = 0.f;
			NewRotation.Roll = 0.f;

			GetController()->SetControlRotation(NewRotation);
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

//Attacking functionality
#pragma region 

void ACharacter1::Attack()
{
	if (PunchMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && !AnimInstance->Montage_IsPlaying(PunchMontage))
		{
			bHasHitThisPunch = false;  // reset each new punch
			IsPunching = true;

			AnimInstance->Montage_Play(PunchMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ACharacter1::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, PunchMontage);

			GetCharacterMovement()->MaxWalkSpeed = 200.0f;
		}
	}
}
//Punch animation finished
void ACharacter1::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PunchMontage)
	{
		IsPunching = false;

	}
}
#pragma endregion

//Blocking functionality
#pragma region 
void ACharacter1::Block()
{
	if (BlockMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && !AnimInstance->Montage_IsPlaying(BlockMontage))
		{
			IsBlocking = true;

			AnimInstance->Montage_Play(BlockMontage);

			// Bind end event
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ACharacter1::OnBlockMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, BlockMontage);

			GetCharacterMovement()->MaxWalkSpeed = 200.0f;
		}
	}
}

//Block animation finished
void ACharacter1::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == BlockMontage)
	{
		IsBlocking = false;
	}
}
#pragma endregion



void ACharacter1::PerformPunchTrace()
{
	if (bHasHitThisPunch) return;

	USkeletalMeshComponent* SkelMesh = GetMesh();  // renamed from Mesh
	if (!SkelMesh) return;

	const FVector TraceStart = SkelMesh->GetBoneLocation(PunchBoneName);
	const FVector TraceEnd = TraceStart + GetActorForwardVector() * PunchTraceLength;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(PunchTraceRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	bool bHit = GetWorld()->SweepMultiByChannel(
		Hits, TraceStart, TraceEnd,
		FQuat::Identity, ECC_Pawn, Sphere, Params
	);

	DrawDebugSphere(GetWorld(), TraceStart, PunchTraceRadius, 8,
		bHit ? FColor::Red : FColor::Green, false, 0.5f);

	if (bHit)
	{
		for (const FHitResult& Hit : Hits)
		{
			ACharacterAI* HitAI = Cast<ACharacterAI>(Hit.GetActor());
			if (HitAI)
			{
				HitAI->health = FMath::Clamp(HitAI->health - damage, 0, 100);
				UE_LOG(LogTemp, Log, TEXT("Hit AI! Health remaining: %d"), HitAI->health);
				bHasHitThisPunch = true;
				break;
			}
		}
	}
}
