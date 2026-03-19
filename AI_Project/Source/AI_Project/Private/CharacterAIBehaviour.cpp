#include "CharacterAIBehaviour.h"
#include "CharacterAI.h"  
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Actor.h"

ACharacterAIBehaviour::ACharacterAIBehaviour()
{
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ACharacterAIBehaviour::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ACharacterAI* AICharacter = Cast<ACharacterAI>(InPawn);
    if (AICharacter && AICharacter->BehaviorTreeAsset)
    {
        BlackboardComponent->InitializeBlackboard(*AICharacter->BehaviorTreeAsset->BlackboardAsset);
        BehaviorTreeComponent->StartTree(*AICharacter->BehaviorTreeAsset);

        // Optionally set blackboard values here, e.g. target player
        if (AICharacter->TargetPlayer)
        {
            BlackboardComponent->SetValueAsObject("TargetPlayer", AICharacter->TargetPlayer);
        }
    }
}