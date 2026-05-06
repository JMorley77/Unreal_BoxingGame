#include "CharacterAI.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "Character1.h"
#include "DrawDebugHelpers.h"
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

}

void ACharacterAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetPlayer) return;

    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // Only move if not mid-action
    if (!bIsRetreating && !bIsBlocking)
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
    FRotator CurrentRotation = GetActorRotation();
    FRotator TargetRotation = FRotator(0.f, LookAt.Yaw, 0.f);

    FRotator SmoothRotation = FMath::RInterpTo(
        CurrentRotation,
        TargetRotation,
        GetWorld()->GetDeltaSeconds(),
        8.f
    );

    SetActorRotation(SmoothRotation);
}

void ACharacterAI::Attack()
{
    UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInst || !PunchMontage) return;
    if (IsExhausted()) return;
    if (bIsAttacking) return;

    if (AnimInst->Montage_IsPlaying(BlockMontage))
    {
        AnimInst->Montage_Stop(0.15f, BlockMontage);
        bIsBlocking = false;
    }

    AnimInst->OnMontageEnded.RemoveDynamic(this, &ACharacterAI::OnAttackMontageEnded);
    AnimInst->OnMontageEnded.AddDynamic(this, &ACharacterAI::OnAttackMontageEnded);

    bHasHitThisPunch = false;  // reset each new punch
    AnimInst->Montage_Play(PunchMontage);
    bIsAttacking = true;
    bIsBlocking = false;
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
    if (!TargetPlayer || !GetWorld())
    {
        return;
    }

    const float Time = GetWorld()->GetTimeSeconds();

    if (Time - LastDecisionTime < DecisionInterval)
    {
        return;
    }

    LastDecisionTime = Time;

    const float StaminaRatio = CurrentStamina / MaxStamina;
    const float HealthRatio = static_cast<float>(health) / 100.f;
    const float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    float AggressiveScore = 0.f;
    float DefensiveScore = 0.f;
    float CounterScore = 0.f;
    float RecoverScore = 0.f;

    // Aggressive: best when healthy, energetic, and close enough.
    AggressiveScore += StaminaRatio * 45.f;
    AggressiveScore += HealthRatio * 20.f;

    if (DistanceToPlayer < 180.f)
    {
        AggressiveScore += 25.f;
    }

    if (bPlayerIsBlocking)
    {
        AggressiveScore += 20.f;
    }

    if (bPlayerIsAttacking)
    {
        DefensiveScore += 90.f;
        CounterScore += 110.f;
        AggressiveScore -= 50.f;
    }
    else
    {
        DefensiveScore += 10.f;
    }

    // Defensive: useful when player is attacking or AI is under pressure.
    DefensiveScore += bPlayerIsAttacking ? 60.f : 10.f;
    DefensiveScore += IsLowStamina() ? 20.f : 0.f;

    // Counter: best when the player attacks and AI has enough stamina.
    CounterScore += bPlayerIsAttacking ? 70.f : 0.f;
    CounterScore += StaminaRatio > 0.45f ? 25.f : -20.f;

    // Recover: best when tired or hurt.
    RecoverScore += IsExhausted() ? 100.f : 0.f;
    RecoverScore += StaminaRatio < 0.35f ? 45.f : 0.f;
    RecoverScore += HealthRatio < 0.35f ? 30.f : 0.f;

    if (RecoverScore >= AggressiveScore &&
        RecoverScore >= DefensiveScore &&
        RecoverScore >= CounterScore)
    {
        CurrentStyle = ECombatStyle::Recovering;
    }
    else if (CounterScore >= AggressiveScore &&
        CounterScore >= DefensiveScore)
    {
        CurrentStyle = ECombatStyle::Counter;
    }
    else if (DefensiveScore >= AggressiveScore)
    {
        CurrentStyle = ECombatStyle::Defensive;
    }
    else
    {
        CurrentStyle = ECombatStyle::Aggressive;
    }
}


void ACharacterAI::PerformPunchTrace()
{
    if (bHasHitThisPunch) return;

    USkeletalMeshComponent* SkelMesh = GetMesh();
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
            ACharacter1* HitPlayer = Cast<ACharacter1>(Hit.GetActor());
            if (HitPlayer)
            {
                HitPlayer->health = FMath::Clamp(HitPlayer->health - damage, 0, 100);
                UE_LOG(LogTemp, Log, TEXT("AI hit player! Player health: %d"), HitPlayer->health);
                bHasHitThisPunch = true;
                break;
            }
        }
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