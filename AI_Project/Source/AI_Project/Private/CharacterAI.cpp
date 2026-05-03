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
    GetCharacterMovement()->MaxWalkSpeed = moveSpeed;

    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ACharacterAI::BeginPlay()
{
    Super::BeginPlay();

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        TargetPlayer = PlayerPawn;
    }

    GetCharacterMovement()->MaxWalkSpeed = moveSpeed;

    if (BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
    {
        BlackboardComponent->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);

        if (TargetPlayer)
        {
            BlackboardComponent->SetValueAsObject("TargetPlayer", TargetPlayer);
        }

        BehaviorTreeComponent->StartTree(*BehaviorTreeAsset);
    }
}

void ACharacterAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetPlayer) return;

    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // Only move if not mid-action
    if (!bIsRetreating && !bIsAttacking && !bIsBlocking)
    {
        MoveTowardsPlayer(TargetPlayer);
    }

    // Speed management based on distance and state
    if (Distance < 550.f)
    {
        if (bIsRetreating)
            GetCharacterMovement()->MaxWalkSpeed = 140.f;
        else if (bIsAttacking || bIsBlocking)
            GetCharacterMovement()->MaxWalkSpeed = 100.f;
        else
            GetCharacterMovement()->MaxWalkSpeed = 250.f;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = moveSpeed;
        bInCombat = false;
    }

    RegenerateStamina(DeltaTime);

    // Clear retreat when stamina has recovered and not exhausted
    if (bIsRetreating && !IsExhausted())
    {
        bIsRetreating = false;
        GetCharacterMovement()->GroundFriction = 8.f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
    }

    // Tick down any pending delay
    if (PendingActionDelay > 0.f)
    {
        PendingActionDelay -= DeltaTime;
    }
}

void ACharacterAI::MoveTowardsPlayer(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    FVector ToPlayer = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Strafe = FVector::CrossProduct(ToPlayer, FVector::UpVector).GetSafeNormal();
    FVector MoveDir = (ToPlayer * 0.85f + Strafe * 0.15f).GetSafeNormal();

    AddMovementInput(MoveDir, 1.0f);

    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerActor->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void ACharacterAI::Attack()
{
    UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInst || !PunchMontage) return;
    if (IsExhausted()) return;
    if (bIsAttacking) return;

    // Force clear blocking state — if block montage is done or nearly done, allow attack
    if (AnimInst->Montage_IsPlaying(BlockMontage))
    {
        AnimInst->Montage_Stop(0.15f, BlockMontage); // blend out cleanly
        bIsBlocking = false;
    }

    AnimInst->OnMontageEnded.RemoveDynamic(this, &ACharacterAI::OnAttackMontageEnded);
    AnimInst->OnMontageEnded.AddDynamic(this, &ACharacterAI::OnAttackMontageEnded);

    AnimInst->Montage_Play(PunchMontage);
    bIsAttacking = true;
    bIsBlocking = false; // ensure cleared
    bInCombat = true;
    ConsumeStamina(PunchCost);
    GetCharacterMovement()->MaxWalkSpeed = 100.f;
}

void ACharacterAI::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == PunchMontage)
    {
        bIsAttacking = false;
        // Restore movement speed after attack finishes
        GetCharacterMovement()->MaxWalkSpeed = bIsRetreating ? 140.f : moveSpeed;
    }
}

void ACharacterAI::Block()
{
    UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInst || !BlockMontage) return;
    if (IsExhausted() || bIsAttacking || bIsBlocking) return;
    if (AnimInst->Montage_IsPlaying(BlockMontage)) return;

    AnimInst->OnMontageEnded.RemoveDynamic(this, &ACharacterAI::OnBlockMontageEnded);
    AnimInst->OnMontageEnded.AddDynamic(this, &ACharacterAI::OnBlockMontageEnded);

    AnimInst->Montage_Play(BlockMontage);
    bIsBlocking = true;
    bInCombat = true;
    ConsumeStamina(BlockCost);
    GetCharacterMovement()->MaxWalkSpeed = 100.f;
}

void ACharacterAI::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == BlockMontage)
    {
        bIsBlocking = false;
        GetCharacterMovement()->MaxWalkSpeed = bIsRetreating ? 140.f : moveSpeed;
    }
}

void ACharacterAI::Retreat()
{
    if (!TargetPlayer) return;

    bInCombat = false;
    bIsRetreating = true;

    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPlayer->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

    FVector Away = (GetActorLocation() - TargetPlayer->GetActorLocation()).GetSafeNormal();
    FVector Strafe = FVector::CrossProduct(Away, FVector::UpVector).GetSafeNormal();
    FVector RetreatDir = (Away * 0.6f + Strafe * 0.4f).GetSafeNormal();

    GetCharacterMovement()->MaxWalkSpeed = 140.f;
    GetCharacterMovement()->GroundFriction = 2.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 200.f;
    AddMovementInput(RetreatDir, 1.0f);
}

void ACharacterAI::UpdateCombatStyle()
{
    float Time = GetWorld()->GetTimeSeconds();
    float StaminaRatio = CurrentStamina / MaxStamina;

    // Handle Recovering exit conditions first
    if (CurrentStyle == ECombatStyle::Recovering)
    {
        bool bRecovered = StaminaRatio >= RecoveryExitStaminaRatio;
        bool bTimedOut = (Time - RecoveryStartTime) >= MaxRecoveryTime;
        if (!bRecovered && !bTimedOut) return;
        // Fall through to re-roll
    }
    else
    {
        if (Time - LastDecisionTime < DecisionInterval) return;
    }

    LastDecisionTime = Time;
    ReactionTime = FMath::FRandRange(0.1f, 0.4f);

    // Enter Recovering only when truly exhausted
    if (IsExhausted())
    {
        if (CurrentStyle != ECombatStyle::Recovering)
            RecoveryStartTime = Time;

        CurrentStyle = ECombatStyle::Recovering;
        return;
    }

    // Weighted style selection based on stamina
    float Rand = FMath::FRand();

    if (StaminaRatio < 0.45f)
    {
        // Low stamina — never aggressive
        CurrentStyle = Rand < 0.55f ? ECombatStyle::Defensive : ECombatStyle::Counter;
    }
    else if (StaminaRatio > 0.75f)
    {
        // High stamina — can be aggressive
        if (Rand < 0.5f)       CurrentStyle = ECombatStyle::Aggressive;
        else if (Rand < 0.8f)  CurrentStyle = ECombatStyle::Defensive;
        else                   CurrentStyle = ECombatStyle::Counter;
    }
    else
    {
        // Mid stamina — balanced
        if (Rand < 0.3f)       CurrentStyle = ECombatStyle::Aggressive;
        else if (Rand < 0.65f) CurrentStyle = ECombatStyle::Defensive;
        else                   CurrentStyle = ECombatStyle::Counter;
    }
}

void ACharacterAI::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
}

void ACharacterAI::RegenerateStamina(float DeltaTime)
{
    float Rate = bInCombat ? CombatRegenRate : RegenRate;
    CurrentStamina = FMath::Clamp(CurrentStamina + Rate * DeltaTime, 0.f, MaxStamina);
}

bool ACharacterAI::IsExhausted() const
{
    return CurrentStamina <= ExhaustedStaminaThreshold;
}

bool ACharacterAI::IsLowStamina() const
{
    return CurrentStamina <= LowStaminaThreshold;
}