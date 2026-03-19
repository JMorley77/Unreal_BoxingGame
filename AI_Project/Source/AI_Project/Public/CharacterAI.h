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
    float MoveSpeed = 400.f;

    // get the player reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetPlayer;

    // punch animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* PunchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* BlockMontage;

    // combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int Health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int Damage = 20;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;
};