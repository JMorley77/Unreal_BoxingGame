#include "BTTask_Block.h"
#include "AIController.h"
#include "CharacterAI.h"

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

    MyPawn->Block();

    return EBTNodeResult::Succeeded;
}