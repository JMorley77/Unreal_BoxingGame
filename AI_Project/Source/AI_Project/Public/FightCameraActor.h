#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "FightCameraActor.generated.h"

UCLASS()
class AI_PROJECT_API AFightCameraActor : public AActor
{
    GENERATED_BODY()

public:
    AFightCameraActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Fighters to track
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    AActor* Fighter1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    AActor* Fighter2;

    // Camera components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCameraComponent* Camera;

    // Camera settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float MinDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float MaxDistance = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float PositionInterpSpeed = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float RotationInterpSpeed = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float DefaultPitch = -10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float BackOffsetDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float SideOffsetDistance = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fight Camera")
    float HeightOffset = 200.f;
};