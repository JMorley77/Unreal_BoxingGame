#include "CharacterAIBehaviour.h"
#include "CharacterAI.h"  
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/Character.h"
#include "Character1.h"

ACharacterAIBehaviour::ACharacterAIBehaviour()
{
    PrimaryActorTick.bCanEverTick = true;

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

        // Get the actual player pawn at runtime
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            BlackboardComponent->SetValueAsObject(TargetPlayerKey, PlayerPawn);
        }
    }
}

void ACharacterAIBehaviour::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APawn* AIPawn = GetPawn();
    if (!AIPawn || !BlackboardComponent) return;

    // Get player ONCE
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    // Update target
    BlackboardComponent->SetValueAsObject(TargetPlayerKey, PlayerPawn);

    // Distance calculation
    float Distance = FVector::Dist(
        AIPawn->GetActorLocation(),
        PlayerPawn->GetActorLocation()
    );

    BlackboardComponent->SetValueAsFloat(DistanceKey, Distance);

    // Cast to your player class
    ACharacter1* Player = Cast<ACharacter1>(PlayerPawn);
    if (Player)
    {
        BlackboardComponent->SetValueAsBool("PlayerAttacking", Player->IsPunching);
        BlackboardComponent->SetValueAsBool("PlayerBlocking", Player->IsBlocking);
    }

    ACharacterAI* AIChar = Cast<ACharacterAI>(AIPawn);
    if (AIChar)
    {
        bool bExhausted = AIChar->IsExhausted();
        float StaminaRatio = AIChar->CurrentStamina / AIChar->MaxStamina;

        BlackboardComponent->SetValueAsBool("Exhausted", bExhausted);
        BlackboardComponent->SetValueAsFloat("StaminaRatio", StaminaRatio);
    }
}