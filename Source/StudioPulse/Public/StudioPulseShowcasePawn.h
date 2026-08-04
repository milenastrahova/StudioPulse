#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StudioPulseShowcasePawn.generated.h"

class UCameraComponent;
class UInputComponent;
class USceneComponent;

UCLASS()
class STUDIOPULSE_API AStudioPulseShowcasePawn
    : public APawn
{
    GENERATED_BODY()

public:
    AStudioPulseShowcasePawn();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent
    ) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StudioPulse|Camera")
    FVector LookAtTarget = FVector(0.0f, 0.0f, 620.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StudioPulse|Camera")
    float SwayAmplitude = 115.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StudioPulse|Camera")
    float SwaySpeed = 0.24f;

private:
    void SetLiveMode();
    void SetDemoMode();
    void SetWarningMode();
    void SetOfflineMode();
    void SetAutomaticMode();

    FVector InitialLocation;
    float ElapsedTime = 0.0f;
};