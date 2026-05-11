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

protected:
	virtual void BeginPlay() override;

public:
	ACharacter1();
	virtual void Tick(float DeltaTime) override;


#pragma region Input and Combat actions
protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Attack();
	void Block();	
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformPunchTrace();

	// Rotation speed when turning toward AI
	UPROPERTY(EditAnywhere, Category = "Combat")
	float RotationSpeed = 10.f;
#pragma endregion

#pragma region Montages
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* PunchMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* BlockMontage;
#pragma endregion

#pragma region States
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool IsPunching = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool IsBlocking = false;
#pragma endregion

#pragma region Stats
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int health = 100;
	int damage = 20;
#pragma endregion

#pragma region Trace Settings
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName PunchBoneName = TEXT("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float PunchTraceRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float PunchTraceLength = 40.f;
#pragma endregion


private:
	bool bHasHitThisPunch = false;

	// Tracks the nearest AI opponent
	ACharacterAI* TargetAI = nullptr;
};