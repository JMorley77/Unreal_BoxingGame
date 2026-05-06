// AnimNotify_PunchHit.h
#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PunchHit.generated.h"

UCLASS()
class AI_PROJECT_API UAnimNotify_PunchHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation) override;
};