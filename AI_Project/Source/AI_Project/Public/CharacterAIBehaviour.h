#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CharacterAIBehaviour.generated.h"

UCLASS()
class AI_PROJECT_API ACharacterAIBehaviour : public AAIController
{
    GENERATED_BODY()

public:
    ACharacterAIBehaviour();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    // Behavior Tree components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    bool bPlayerIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    bool bPlayerIsBlocking = false;

    FName TargetPlayerKey = "TargetPlayer";
    FName DistanceKey = "DistanceToPlayer";
};