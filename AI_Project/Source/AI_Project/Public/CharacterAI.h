#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CharacterAI.generated.h"

UENUM(BlueprintType)
enum class ECombatStyle : uint8
{
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Counter     UMETA(DisplayName = "Counter"),
    Recovering  UMETA(DisplayName = "Recovering")
};

class UAnimMontage;

UCLASS()
class AI_PROJECT_API ACharacterAI : public ACharacter
{
    GENERATED_BODY()

public:
    ACharacterAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void MoveTowardsPlayer(AActor* PlayerActor);
    void UpdateCombatStyle();
    void Attack();
    void Block();
    void Retreat();

    UFUNCTION()
    void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float moveSpeed = 400.f;

    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* PunchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 damage = 20;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking = false;

    // Stamina
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float MaxStamina = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    float CurrentStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float RegenRate = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float CombatRegenRate = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float ExhaustedStaminaThreshold = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float LowStaminaThreshold = 40.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    bool bInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
    bool bIsRetreating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float PunchCost = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float BlockCost = 10.f;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RegenerateStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsExhausted() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsLowStamina() const;

    // AI Style
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    ECombatStyle CurrentStyle = ECombatStyle::Aggressive;

    UPROPERTY(EditAnywhere, Category = "AI")
    float DecisionInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "AI")
    float RecoveryExitStaminaRatio = 0.5f;

    UPROPERTY(EditAnywhere, Category = "AI")
    float MaxRecoveryTime = 4.0f;

    float LastDecisionTime = 0.f;
    float RecoveryStartTime = 0.f;
    float ReactionTime = 0.f;
    float PendingActionDelay = 0.f;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;
};