#include "BTTask_Attack.h"
#include "AIController.h"
#include "CharacterAI.h"

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

    MyPawn->Attack();//calls the attack function 

    return EBTNodeResult::Succeeded;
}