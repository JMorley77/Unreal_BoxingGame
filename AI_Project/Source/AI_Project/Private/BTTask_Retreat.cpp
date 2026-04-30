#include "BTTask_Retreat.h"
#include "AIController.h"
#include "CharacterAI.h"

UBTTask_Retreat::UBTTask_Retreat()
{
    NodeName = "Execute Retreat";
    bNotifyTick = true;  // enables TickTask
}

EBTNodeResult::Type UBTTask_Retreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;  //  keep running
}

void UBTTask_Retreat::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    if (!MyController) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    ACharacterAI* MyPawn = Cast<ACharacterAI>(MyController->GetPawn());
    if (!MyPawn) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    // Stop retreating once stamina recovers
    if (!MyPawn->IsExhausted())
    {
        MyPawn->bIsRetreating = false;
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    MyPawn->Retreat();  //  called every frame now
}