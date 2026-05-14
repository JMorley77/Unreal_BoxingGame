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
    // caches the player at the start
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerPawn)
    {
        TargetPlayer = PlayerPawn;
    }

    GetCharacterMovement()->MaxWalkSpeed = moveSpeed;

}

#pragma region Tick
void ACharacterAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetPlayer) return;

    float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

    // only move if not mid action
    if (!bIsRetreating && !bIsBlocking)
    {
        MoveTowardsPlayer(TargetPlayer);
    }

    // speed management based on distance and state
    //550 is in combat range, change to adjust how close the ai needs to be befeor attacking
    if (Distance < 550.f)
    {
        if (bIsRetreating)
            GetCharacterMovement()->MaxWalkSpeed = 140.f;
        else if (bIsAttacking || bIsBlocking)
            //slow during actions
            GetCharacterMovement()->MaxWalkSpeed = 100.f;
        else
            //slows down when closer to player 
            GetCharacterMovement()->MaxWalkSpeed = 250.f;
    }
    else
    {
		// when out of combat range, return to normal speed
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

    //tick down any pending delay
    if (PendingActionDelay > 0.f)
    {
        PendingActionDelay -= DeltaTime;
    }
}
#pragma endregion


#pragma region Movement
void ACharacterAI::MoveTowardsPlayer(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    // small strafe so AI doesnt walk directly at player
    FVector ToPlayer = (PlayerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Strafe = FVector::CrossProduct(ToPlayer, FVector::UpVector).GetSafeNormal();
    FVector MoveDir = (ToPlayer * 0.85f + Strafe * 0.15f).GetSafeNormal();

    AddMovementInput(MoveDir, 1.0f);
    // smoothly face the player each frame 
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
#pragma endregion


#pragma region Attacking
void ACharacterAI::Attack()
{
    UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInst || !PunchMontage) return;
    if (IsExhausted()) return;
    if (bIsAttacking) return;
	// cancel block if attacking, but not the other way around since you can attack out of a block
    if (AnimInst->Montage_IsPlaying(BlockMontage))
    {
        AnimInst->Montage_Stop(0.15f, BlockMontage);
        bIsBlocking = false;
    }
    // if Attack() is called the previous montage ends
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
#pragma endregion


#pragma region Blocking
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
#pragma endregion


#pragma region Retreat
void ACharacterAI::Retreat()
{
    if (!TargetPlayer) return;

    bInCombat = false;
    bIsRetreating = true;

    // keep facing player until retreating ends
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
#pragma endregion


#pragma region Stamina
void ACharacterAI::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
}

void ACharacterAI::RegenerateStamina(float DeltaTime)
{
    //slower combat rate while in range
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
#pragma endregion


#pragma region Decision Logic
void ACharacterAI::UpdateCombatStyle()
{
    if (!TargetPlayer || !GetWorld())
    {
        return;
    }

    const float Time = GetWorld()->GetTimeSeconds();
    // decisions to DecisionInterval so dont need to update every tiuck
    if (Time - LastDecisionTime < DecisionInterval)
    {
        return;
    }

    LastDecisionTime = Time;

    const float StaminaRatio = CurrentStamina / MaxStamina;
    const float HealthRatio = static_cast<float>(health) / 100.f;
    const float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
    // Each style accumulates a score and the highest score wins 
    // adjust these values to change AI personality:
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

    // Defensive: Heaviely used when player is attacking
    DefensiveScore += bPlayerIsAttacking ? 60.f : 10.f;
    DefensiveScore += IsLowStamina() ? 20.f : 0.f;

    // Counter: same as defensive but needs more stamina to follow through
    CounterScore += bPlayerIsAttacking ? 70.f : 0.f;
    CounterScore += StaminaRatio > 0.45f ? 25.f : -20.f;

    // Recover: activates when stamina or health is low
    RecoverScore += IsExhausted() ? 100.f : 0.f;
    RecoverScore += StaminaRatio < 0.35f ? 45.f : 0.f;
    RecoverScore += HealthRatio < 0.35f ? 30.f : 0.f;

    // picks the highest score
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
#pragma endregion


#pragma region Trace Settings
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
    //visual debug red is hit green is miss
    DrawDebugSphere(GetWorld(), TraceStart, PunchTraceRadius, 8,
        bHit ? FColor::Red : FColor::Green, false, 0.5f);

    if (bHit)
    {
        for (const FHitResult& Hit : Hits)
        {
            ACharacter1* HitPlayer = Cast<ACharacter1>(Hit.GetActor());
            if (HitPlayer)
            {
				//blocking reduces damage to 10% of original
                int32 FinalDamage = damage;

                if (HitPlayer->IsBlocking)
                {
                    FinalDamage = FMath::RoundToInt(damage * 0.1f);
                    UE_LOG(LogTemp, Log, TEXT("Player blocked! Reduced damage: %d"), FinalDamage);
                }

                HitPlayer->health = FMath::Clamp(HitPlayer->health - FinalDamage, 0, 100);
                UE_LOG(LogTemp, Log, TEXT("AI hit player! Player health: %d"), HitPlayer->health);
                bHasHitThisPunch = true;
                break;// stop after first valid hit 
            }
        }
    }
}
#pragma endregion
















