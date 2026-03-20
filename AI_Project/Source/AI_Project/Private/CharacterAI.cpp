#include "CharacterAI.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

ACharacterAI::ACharacterAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // set movement speed
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    // create behavior tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ACharacterAI::BeginPlay()
{
    Super::BeginPlay();

    // find player when the game begins
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        TargetPlayer = PlayerPawn;
    }

    // sets the ai speed
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    // Start Behavior Tree
    if (BehaviorTreeAsset)
    {
        if (BehaviorTreeAsset->BlackboardAsset)
        {
            BlackboardComponent->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
        }

        // Set TargetPlayer in blackboard
        if (BlackboardComponent && TargetPlayer)
        {
            BlackboardComponent->SetValueAsObject("TargetPlayer", TargetPlayer);
        }

        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
    }
}

void ACharacterAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (TargetPlayer)
    {
        MoveTowardsPlayer(TargetPlayer);

        float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
        if (Distance < 550.f)
        {
            GetCharacterMovement()->MaxWalkSpeed = 250.f; // slow down when close to player
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; // normal speed
        }
    }
}

void ACharacterAI::MoveTowardsPlayer(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    FVector Direction = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction, 1.0f);

    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerActor->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
}

void ACharacterAI::Attack()
{
    if (PunchMontage && GetMesh() && GetMesh()->GetAnimInstance() && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(PunchMontage))
    {
        GetMesh()->GetAnimInstance()->Montage_Play(PunchMontage);
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
    }
}

void ACharacterAI::Block()
{
    if (BlockMontage && GetMesh() && GetMesh()->GetAnimInstance() && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(BlockMontage))
    {
        GetMesh()->GetAnimInstance()->Montage_Play(BlockMontage);
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
    }
}