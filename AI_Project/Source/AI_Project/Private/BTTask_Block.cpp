#include "BTTask_Block.h"
#include "AIController.h"
#include "CharacterAI.h"
// Replace with your actual character header

UBTTask_Block::UBTTask_Block()
{
    NodeName = "Execute Block";
}

EBTNodeResult::Type UBTTask_Block::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    if (!MyController) return EBTNodeResult::Failed;

    ACharacterAI* MyPawn = Cast<ACharacterAI>(MyController->GetPawn());
    if (!MyPawn) return EBTNodeResult::Failed;

    // Call your existing attack function
    MyPawn->Block();

    // Return Succeeded so the tree knows the task is done
    return EBTNodeResult::Succeeded;
}
