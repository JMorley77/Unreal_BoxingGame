#include "FightCameraActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

AFightCameraActor::AFightCameraActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Spring Arm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 10.f;

    // Create Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
}

void AFightCameraActor::BeginPlay()
{
    Super::BeginPlay();

    // Assign Fighter1 if not set
    if (!Fighter1)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            if (PC->GetPawn())
            {
                Fighter1 = PC->GetPawn();
            }
        }
    }

    // Assign Fighter2 if not set
    if (!Fighter2)
    {
        TArray<AActor*> Pawns;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Pawns);

        for (AActor* PawnActor : Pawns)
        {
            if (PawnActor && PawnActor != Fighter1)
            {
                Fighter2 = PawnActor;
                break;
            }
        }
    }

    // Set this camera as view target
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetViewTarget(this);
    }
}

void AFightCameraActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!Fighter1 || !Fighter2) return;

    //midpoint between fighters
    FVector Midpoint = (Fighter1->GetActorLocation() + Fighter2->GetActorLocation()) / 2;

    //direction from Fighter1 to Fighter2
    FVector Direction = (Fighter2->GetActorLocation() - Fighter1->GetActorLocation()).GetSafeNormal();

    //camera offsets
    FVector BackOffset = -Direction * BackOffsetDistance;          // behind fighters
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    FVector SideOffset = RightVector * SideOffsetDistance;         // slightly to side
    FVector UpOffset = FVector(0, 0, HeightOffset);               // above fighters

    FVector TargetLocation = Midpoint + BackOffset + SideOffset + UpOffset;

    //smoothly move camera
    FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, PositionInterpSpeed);
    SetActorLocation(NewLocation);

    //adjust SpringArm length based on fighter distance
    float Distance = FVector::Dist(Fighter1->GetActorLocation(), Fighter2->GetActorLocation());
    float TargetArmLength = FMath::Clamp(Distance, MinDistance, MaxDistance);
    SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, PositionInterpSpeed);

    //smoothly rotate camera towards the midpoint
    FVector LookTarget = Midpoint + UpOffset * 0.5f; // look slightly above midpoint
    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookTarget);
    TargetRotation.Pitch += DefaultPitch;
    FRotator SmoothRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed);
    SetActorRotation(SmoothRotation);
}