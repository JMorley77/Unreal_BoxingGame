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
    GetCharacterMovement()->MaxWalkSpeed = moveSpeed;

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
    GetCharacterMovement()->MaxWalkSpeed = moveSpeed;

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
        if (!bIsRetreating) 
        {
            MoveTowardsPlayer(TargetPlayer);
        }

        float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
        if (Distance < 550.f)
        {
            GetCharacterMovement()->MaxWalkSpeed = bIsRetreating ? 140.f : 250.f;  //retreat speed
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = moveSpeed;
            bInCombat = false;
        }
    }

    RegenerateStamina(DeltaTime);

    if (bIsRetreating && !IsExhausted())
    {
        bIsRetreating = false;
        GetCharacterMovement()->GroundFriction = 8.f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
    }
}

void ACharacterAI::MoveTowardsPlayer(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    FVector ToPlayer = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Strafe = FVector::CrossProduct(ToPlayer, FVector::UpVector).GetSafeNormal();

    // Slight sideways drift while approaching
    FVector MoveDir = (ToPlayer * 0.85f + Strafe * 0.15f).GetSafeNormal();
    AddMovementInput(MoveDir, 1.0f);

    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerActor->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
}

void ACharacterAI::Attack()
{
    if (!IsExhausted() && PunchMontage && GetMesh() && GetMesh()->GetAnimInstance() && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(PunchMontage))
    {
        GetMesh()->GetAnimInstance()->Montage_Play(PunchMontage);
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
		ConsumeStamina(PunchCost);
        bInCombat = true;
    }
}

void ACharacterAI::Block()
{
    if (!IsExhausted() && BlockMontage && GetMesh() && GetMesh()->GetAnimInstance() && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(BlockMontage))
    {
        GetMesh()->GetAnimInstance()->Montage_Play(BlockMontage);
        GetCharacterMovement()->MaxWalkSpeed = 100.0f;
        ConsumeStamina(BlockCost);
        bInCombat = true;
    }
}

void ACharacterAI::Retreat()
{
    if (!TargetPlayer) return;

    bInCombat = false;
    bIsRetreating = true;

    // Face the player
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPlayer->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));

    // Get direction away from player
    FVector AwayFromPlayer = (GetActorLocation() - TargetPlayer->GetActorLocation()).GetSafeNormal();

    // Get perpendicular (strafe) direction for circling
    FVector StrafeDirection = FVector::CrossProduct(AwayFromPlayer, FVector::UpVector).GetSafeNormal();

    // Mix backing away + strafing sideways
    FVector RetreatDirection = (AwayFromPlayer * 0.6f + StrafeDirection * 0.4f).GetSafeNormal();

    GetCharacterMovement()->MaxWalkSpeed = 140.f;
    GetCharacterMovement()->GroundFriction = 2.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 200.f;
    AddMovementInput(RetreatDirection, 1.0f);
}

void ACharacterAI::UpdateCombatStyle() 
{
    float Time = GetWorld()->GetTimeSeconds();
    if (Time - LastDecisionTime < DecisionInterval) return;

    LastDecisionTime = Time;

    float StaminaRatio = CurrentStamina / MaxStamina;
    float Rand = FMath::FRand();

    // Decision logic (this is the "brain")
    if (StaminaRatio < 0.3f)
    {
        CurrentStyle = ECombatStyle::Recovering;
    }
    else if (Rand < 0.4f)
    {
        CurrentStyle = ECombatStyle::Aggressive;
    }
    else if (Rand < 0.7f)
    {
        CurrentStyle = ECombatStyle::Defensive;
    }
    else 
    {
        CurrentStyle = ECombatStyle::Counter;
    }
}








void ACharacterAI::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
}

void ACharacterAI::RegenerateStamina(float DeltaTime)
{
    float RegenAmount = bInCombat ? CombatRegenRate : RegenRate;
    CurrentStamina = FMath::Clamp(CurrentStamina + RegenAmount * DeltaTime, 0.f, MaxStamina);
}

bool ACharacterAI::IsExhausted() const
{
    return CurrentStamina <= ExhaustedStaminaThreshold;
}

bool ACharacterAI::IsLowStamina() const
{
    return CurrentStamina <= LowStaminaThreshold;
}