#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterAI.generated.h"


class UAnimMontage;
class UBehaviorTree;

UCLASS()
class AI_PROJECT_API ACharacterAI : public ACharacter
{
    GENERATED_BODY()

public:
    ACharacterAI();

protected:
    virtual void BeginPlay() override;

public:

    // Behavior Tree reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    // Combat actions
    UFUNCTION(BlueprintCallable)
    void Attack();

    UFUNCTION(BlueprintCallable)
    void Block();

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed = 400.f;

    // Target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetPlayer;

    // Animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* PunchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* BlockMontage;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int Health = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int Damage = 20;
};