#include "BTTask_Retreat.h"
#include "AIController.h"
#include "CharacterAI.h"

UBTTask_Retreat::UBTTask_Retreat()
{
    NodeName = "Execute Retreat";
}

EBTNodeResult::Type UBTTask_Retreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    if (!MyController) return EBTNodeResult::Failed;

    ACharacterAI* MyPawn = Cast<ACharacterAI>(MyController->GetPawn());
    if (!MyPawn) return EBTNodeResult::Failed;

    MyPawn->Retreat();

    return EBTNodeResult::Succeeded;
}