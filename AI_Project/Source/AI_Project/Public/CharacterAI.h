#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterAI.generated.h"

class UAnimMontage;

UCLASS()
class AI_PROJECT_API ACharacterAI : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values
    ACharacterAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Move to plauyer 
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

	int health = 100;
	int damage = 20;
};
