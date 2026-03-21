#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CharacterAI.generated.h"

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

    // Move to player 
    void MoveTowardsPlayer(AActor* PlayerActor);

    // attack
    void Attack();
    void Block();

    // movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float moveSpeed = 400.f;

    // get the player reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetPlayer;

    // punch animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* PunchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* BlockMontage;

    // combat stats
    #pragma region Combat Stats

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int damage = 20;
    #pragma endregion


	//Stamina stats variables
    #pragma region Stamina Stats Variables

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

    //Costs

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float PunchCost = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float BlockCost = 10.f;
    #pragma endregion

	//Stamina stats functions
    #pragma region Stamina Stats Functions

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RegenerateStamina(float DeltaTime, bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsExhausted() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool IsLowStamina() const;
    #pragma endregion




    //Behaviour Trees
    #pragma region Behaviour Tree

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;
    #pragma endregion


};