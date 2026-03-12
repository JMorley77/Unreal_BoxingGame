#include "CharacterAI.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

ACharacterAI::ACharacterAI()
{
    PrimaryActorTick.bCanEverTick = false;

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ACharacterAI::BeginPlay()
{
    Super::BeginPlay();

    // find player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();

    if (PlayerPawn)
    {
        TargetPlayer = PlayerPawn;
    }

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ACharacterAI::Attack()
{
    if (PunchMontage && GetMesh())
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

        if (AnimInstance && !AnimInstance->Montage_IsPlaying(PunchMontage))
        {
            AnimInstance->Montage_Play(PunchMontage);
        }
    }
}

void ACharacterAI::Block()
{
    if (BlockMontage && GetMesh())
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

        if (AnimInstance && !AnimInstance->Montage_IsPlaying(BlockMontage))
        {
            AnimInstance->Montage_Play(BlockMontage);
        }
    }
}