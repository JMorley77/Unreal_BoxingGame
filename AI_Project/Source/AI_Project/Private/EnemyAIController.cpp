#include "EnemyAIController.h"
#include "CharacterAI.h"
#include "BehaviorTree/BehaviorTree.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ACharacterAI* AICharacter = Cast<ACharacterAI>(InPawn);

    if (AICharacter && AICharacter->BehaviorTreeAsset)
    {
        RunBehaviorTree(AICharacter->BehaviorTreeAsset);

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

        if (PlayerPawn)
        {
            GetBlackboardComponent()->SetValueAsObject("Target", PlayerPawn);
        }
    }
}