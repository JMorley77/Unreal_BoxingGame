#include "CharacterAI.h"
#include "Animation/AnimInstance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACharacterAI::ACharacterAI()
{
    PrimaryActorTick.bCanEverTick = false;

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ACharacterAI::BeginPlay()
{
    Super::BeginPlay();

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();

    if (PlayerPawn)
    {
        TargetPlayer = PlayerPawn;

        AAIController* AIController = Cast<AAIController>(GetController());

        if (AIController && AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->SetValueAsObject(
                TEXT("Target"),
                PlayerPawn
            );
        }
    }
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