#include "BTTask_Attack.h"
#include "AIController.h"
#include "CharacterAI.h"
// Replace with your actual character header

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = "Execute Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    if (!MyController) return EBTNodeResult::Failed;

    ACharacterAI* MyPawn = Cast<ACharacterAI>(MyController->GetPawn());
    if (!MyPawn) return EBTNodeResult::Failed;

    // Call your existing attack function
    MyPawn->Attack();

    // Return Succeeded so the tree knows the task is done
    return EBTNodeResult::Succeeded;
}