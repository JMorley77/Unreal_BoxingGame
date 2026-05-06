// AnimNotify_PunchHit.cpp
#include "AnimNotify_PunchHit.h"
#include "Character1.h"

void UAnimNotify_PunchHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    ACharacter1* Player = Cast<ACharacter1>(MeshComp->GetOwner());
    if (Player)
    {
        Player->PerformPunchTrace();
    }
}