#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CharacterAI.generated.h"


class UAnimMontage;
#pragma region Decision Enum
//decides what decision branch UpdateCombatStyle() will pick
UENUM(BlueprintType)
enum class ECombatStyle : uint8
{
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Counter     UMETA(DisplayName = "Counter"),
    Recovering  UMETA(DisplayName = "Recovering")
};
#pragma endregion

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


#pragma region Combat Actions
    void MoveTowardsPlayer(AActor* PlayerActor);
    void Attack();
    void Block();
    void Retreat();

    void UpdateCombatStyle();

    UFUNCTION()
    void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    // called via an anim notify so damage only registers at the right frame
    // Sphere that gets activated from the punch bone each tick during the attack window;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformPunchTrace();
#pragma endregion

#pragma region States
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bPlayerIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bPlayerIsBlocking = false;

private:
    // Prevents the same punch swing from registering multiple hits; 
    bool bHasHitThisPunch = false;
#pragma endregion

#pragma region Stats
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 damage = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float moveSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetPlayer;

#pragma endregion

#pragma region Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* PunchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* BlockMontage;
#pragma endregion

#pragma region Trace Settings
    // Bone the sphere originates from this maches the bone in skeleton
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName PunchBoneName = TEXT("hand_r"); 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PunchTraceRadius = 20.f;
    // How far forward from the bone the trace extends
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PunchTraceLength = 40.f;
#pragma endregion

#pragma region Stamina
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
#pragma endregion

#pragma region Decision Making
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
#pragma endregion

#pragma region Memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PlayerAttackMemoryTime = 0.8f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Memory")
    float LastSeenPlayerAttackTime = -999.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Memory")
    float LastSeenPlayerBlockTime = -999.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float PlayerActionMemoryTime = 0.8f;
#pragma endregion

#pragma region Behaviour Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;
#pragma endregion










};