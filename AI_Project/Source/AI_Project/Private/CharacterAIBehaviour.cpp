#include "CharacterAIBehaviour.h"
#include "CharacterAI.h"
#include "Character1.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

ACharacterAIBehaviour::ACharacterAIBehaviour()
{
    PrimaryActorTick.bCanEverTick = true;

    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

#pragma region Possession
void ACharacterAIBehaviour::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ACharacterAI* AICharacter = Cast<ACharacterAI>(InPawn);
    if (!AICharacter || !AICharacter->BehaviorTreeAsset)
    {
        return;
    }

    if (AICharacter->BehaviorTreeAsset->BlackboardAsset)
    {
        BlackboardComponent->InitializeBlackboard(*AICharacter->BehaviorTreeAsset->BlackboardAsset);
    }
    //cache the player refernece on the ai and in the blackboard so the tree can use it
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        AICharacter->TargetPlayer = PlayerPawn;
        BlackboardComponent->SetValueAsObject(TargetPlayerKey, PlayerPawn);
    }

    BehaviorTreeComponent->StartTree(*AICharacter->BehaviorTreeAsset);
}
#pragma endregion


#pragma region Tick Perception
void ACharacterAIBehaviour::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APawn* AIPawn = GetPawn();
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (!AIPawn || !PlayerPawn || !BlackboardComponent)
    {
        return;
    }

    ACharacterAI* AICharacter = Cast<ACharacterAI>(AIPawn);
    if (!AICharacter)
    {
        return;
    }

    ACharacter1* PlayerCharacter = Cast<ACharacter1>(PlayerPawn);
    // update target reference 
    AICharacter->TargetPlayer = PlayerPawn;

    const float DistanceToPlayer = FVector::Dist(
        AIPawn->GetActorLocation(),
        PlayerPawn->GetActorLocation()
    );
    // update data to blackbaord every tick 
    BlackboardComponent->SetValueAsObject(TargetPlayerKey, PlayerPawn);
    BlackboardComponent->SetValueAsFloat(DistanceKey, DistanceToPlayer);

    if (PlayerCharacter)
    {
        const float Time = GetWorld()->GetTimeSeconds();
        //strore the last time the AI saw the player do each action
        if (PlayerCharacter->IsPunching)
        {
            AICharacter->LastSeenPlayerAttackTime = Time;
        }

        if (PlayerCharacter->IsBlocking)
        {
            AICharacter->LastSeenPlayerBlockTime = Time;
        }

        const bool bRecentlySawPlayerAttack =
            Time - AICharacter->LastSeenPlayerAttackTime <= AICharacter->PlayerActionMemoryTime;

        const bool bRecentlySawPlayerBlock =
            Time - AICharacter->LastSeenPlayerBlockTime <= AICharacter->PlayerActionMemoryTime;

        AICharacter->bPlayerIsAttacking = bRecentlySawPlayerAttack;
        AICharacter->bPlayerIsBlocking = bRecentlySawPlayerBlock;

        BlackboardComponent->SetValueAsBool(TEXT("PlayerAttacking"), bRecentlySawPlayerAttack);
        BlackboardComponent->SetValueAsBool(TEXT("PlayerBlocking"), bRecentlySawPlayerBlock);
    }
    else
    {
        AICharacter->bPlayerIsAttacking = false;
        AICharacter->bPlayerIsBlocking = false;

        BlackboardComponent->SetValueAsBool(TEXT("PlayerAttacking"), false);
        BlackboardComponent->SetValueAsBool(TEXT("PlayerBlocking"), false);
    }
    // guard against divison by 0
    const float StaminaRatio = AICharacter->MaxStamina > 0.f
        ? AICharacter->CurrentStamina / AICharacter->MaxStamina
        : 0.f;

    BlackboardComponent->SetValueAsBool(TEXT("Exhausted"), AICharacter->IsExhausted());
    BlackboardComponent->SetValueAsFloat(TEXT("StaminaRatio"), StaminaRatio);

    AICharacter->UpdateCombatStyle();
    // UpdateCombatStyle is updated every tick by DecisionInterval
    BlackboardComponent->SetValueAsEnum(
        TEXT("CombatStyle"),
        static_cast<uint8>(AICharacter->CurrentStyle)
    );
}
#pragma endregion


