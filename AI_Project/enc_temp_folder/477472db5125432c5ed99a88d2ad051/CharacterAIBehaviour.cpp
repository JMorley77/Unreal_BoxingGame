#include "CharacterAIBehaviour.h"
#include "CharacterAI.h"  
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h" //  ADD THIS

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

        // FIX: Get the actual player pawn at runtime
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

    // Optional but robust: always ensure target is valid
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        BlackboardComponent->SetValueAsObject(TargetPlayerKey, PlayerPawn);
    }

    AActor* Target = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetPlayerKey));
    if (!Target) return;

    float Distance = FVector::Dist(
        AIPawn->GetActorLocation(),
        Target->GetActorLocation()
    );

    BlackboardComponent->SetValueAsFloat(DistanceKey, Distance);
}