#include "StudioPulseShowcasePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "StudioPulseDataSubsystem.h"

AStudioPulseShowcasePawn::AStudioPulseShowcasePawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot")
        );

    SetRootComponent(SceneRoot);

    Camera =
        CreateDefaultSubobject<UCameraComponent>(
            TEXT("Camera")
        );

    Camera->SetupAttachment(SceneRoot);
    Camera->SetFieldOfView(55.0f);
}

void AStudioPulseShowcasePawn::BeginPlay()
{
    Super::BeginPlay();
    InitialLocation = GetActorLocation();
}

void AStudioPulseShowcasePawn::Tick(
    float DeltaSeconds
)
{
    Super::Tick(DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    const FVector NewLocation =
        InitialLocation +
        FVector(
            FMath::Sin(
                ElapsedTime * SwaySpeed * 0.55f
            ) * 28.0f,
            FMath::Sin(
                ElapsedTime * SwaySpeed
            ) * SwayAmplitude,
            FMath::Sin(
                ElapsedTime * SwaySpeed * 0.37f
            ) * 18.0f
        );

    SetActorLocation(NewLocation);

    const FRotator LookRotation =
        (LookAtTarget - NewLocation).Rotation();

    SetActorRotation(LookRotation);
}


void AStudioPulseShowcasePawn::
SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(
        PlayerInputComponent
    );

    if (!PlayerInputComponent)
    {
        return;
    }

    PlayerInputComponent->BindKey(
        EKeys::One,
        IE_Pressed,
        this,
        &AStudioPulseShowcasePawn::SetLiveMode
    );

    PlayerInputComponent->BindKey(
        EKeys::Two,
        IE_Pressed,
        this,
        &AStudioPulseShowcasePawn::SetWarningMode
    );

    PlayerInputComponent->BindKey(
        EKeys::Three,
        IE_Pressed,
        this,
        &AStudioPulseShowcasePawn::SetOfflineMode
    );

    PlayerInputComponent->BindKey(
        EKeys::Four,
        IE_Pressed,
        this,
        &AStudioPulseShowcasePawn::SetDemoMode
    );

    PlayerInputComponent->BindKey(
        EKeys::Five,
        IE_Pressed,
        this,
        &AStudioPulseShowcasePawn::SetAutomaticMode
    );
}

void AStudioPulseShowcasePawn::SetLiveMode()
{
    if (UGameInstance* GameInstance =
        GetGameInstance())
    {
        if (UStudioPulseDataSubsystem* DataSubsystem =
            GameInstance->GetSubsystem<
                UStudioPulseDataSubsystem
            >())
        {
            DataSubsystem->SetPreviewMode(
                EStudioPulsePreviewMode::Live
            );
        }
    }
}

void AStudioPulseShowcasePawn::SetDemoMode()
{
    if (UGameInstance* GameInstance =
        GetGameInstance())
    {
        if (UStudioPulseDataSubsystem* DataSubsystem =
            GameInstance->GetSubsystem<
                UStudioPulseDataSubsystem
            >())
        {
            DataSubsystem->SetPreviewMode(
                EStudioPulsePreviewMode::Demo
            );
        }
    }
}

void AStudioPulseShowcasePawn::SetWarningMode()
{
    if (UGameInstance* GameInstance =
        GetGameInstance())
    {
        if (UStudioPulseDataSubsystem* DataSubsystem =
            GameInstance->GetSubsystem<
                UStudioPulseDataSubsystem
            >())
        {
            DataSubsystem->SetPreviewMode(
                EStudioPulsePreviewMode::Warning
            );
        }
    }
}

void AStudioPulseShowcasePawn::SetOfflineMode()
{
    if (UGameInstance* GameInstance =
        GetGameInstance())
    {
        if (UStudioPulseDataSubsystem* DataSubsystem =
            GameInstance->GetSubsystem<
                UStudioPulseDataSubsystem
            >())
        {
            DataSubsystem->SetPreviewMode(
                EStudioPulsePreviewMode::Offline
            );
        }
    }
}

void AStudioPulseShowcasePawn::SetAutomaticMode()
{
    if (UGameInstance* GameInstance =
        GetGameInstance())
    {
        if (UStudioPulseDataSubsystem* DataSubsystem =
            GameInstance->GetSubsystem<
                UStudioPulseDataSubsystem
            >())
        {
            DataSubsystem->SetPreviewMode(
                EStudioPulsePreviewMode::Automatic
            );
        }
    }
}