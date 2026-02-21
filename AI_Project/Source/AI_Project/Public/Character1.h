// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <CharacterAI.h>
#include "Character1.generated.h"

class UAnimMontage;

UCLASS()
class AI_PROJECT_API ACharacter1 : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacter1();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	int health = 100;
	int damage = 20;


protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Attack();
	void Block();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* PunchMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* BlockMontage;

private:
	// Tracks the nearest AI opponent
	ACharacterAI* TargetAI = nullptr;

	// Rotation speed when turning toward AI
	UPROPERTY(EditAnywhere, Category = "Combat")
	float RotationSpeed = 10.f;
};