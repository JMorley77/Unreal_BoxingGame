#include "AnimNotify_PunchHit.h"
#include "Character1.h"
#include "CharacterAI.h"

void UAnimNotify_PunchHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();

    if (ACharacter1* Player = Cast<ACharacter1>(Owner))
    {
        Player->PerformPunchTrace();
    }
    else if (ACharacterAI* AI = Cast<ACharacterAI>(Owner))
    {
        AI->PerformPunchTrace();
    }
}