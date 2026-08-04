#include "StudioPulseDisplayActor.h"

#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "StudioPulseDashboardWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

AStudioPulseDisplayActor::AStudioPulseDisplayActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot")
        );

    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh>
        CubeMeshFinder(
            TEXT("/Engine/BasicShapes/Cube.Cube")
        );

    UStaticMesh* CubeMesh =
        CubeMeshFinder.Succeeded()
        ? CubeMeshFinder.Object
        : nullptr;

    auto CreateCubeComponent =
        [this, CubeMesh](
            const FName Name,
            const FVector& Location,
            const FVector& Scale
        )
        {
            UStaticMeshComponent* Component =
                CreateDefaultSubobject<UStaticMeshComponent>(
                    Name
                );

            Component->SetupAttachment(SceneRoot);

            if (CubeMesh)
            {
                Component->SetStaticMesh(CubeMesh);
            }

            Component->SetRelativeLocation(Location);
            Component->SetRelativeScale3D(Scale);
            Component->SetCollisionEnabled(
                ECollisionEnabled::NoCollision
            );

            return Component;
        };

    ScreenBack =
        CreateCubeComponent(
            TEXT("ScreenBack"),
            FVector::ZeroVector,
            FVector(0.10f, 9.90f, 5.65f)
        );

    FrameTop =
        CreateCubeComponent(
            TEXT("FrameTop"),
            FVector(4.0f, 0.0f, 595.0f),
            FVector(0.16f, 10.40f, 0.30f)
        );

    FrameBottom =
        CreateCubeComponent(
            TEXT("FrameBottom"),
            FVector(4.0f, 0.0f, -595.0f),
            FVector(0.16f, 10.40f, 0.30f)
        );

    FrameLeft =
        CreateCubeComponent(
            TEXT("FrameLeft"),
            FVector(4.0f, -1025.0f, 0.0f),
            FVector(0.16f, 0.30f, 6.00f)
        );

    FrameRight =
        CreateCubeComponent(
            TEXT("FrameRight"),
            FVector(4.0f, 1025.0f, 0.0f),
            FVector(0.16f, 0.30f, 6.00f)
        );

    AccentLeft =
        CreateCubeComponent(
            TEXT("AccentLeft"),
            FVector(20.0f, -965.0f, 0.0f),
            FVector(0.05f, 0.04f, 5.40f)
        );

    AccentRight =
        CreateCubeComponent(
            TEXT("AccentRight"),
            FVector(20.0f, 965.0f, 0.0f),
            FVector(0.05f, 0.04f, 5.40f)
        );

    HeaderGlow =
        CreateCubeComponent(
            TEXT("HeaderGlow"),
            FVector(22.0f, 0.0f, 555.0f),
            FVector(0.05f, 9.45f, 0.035f)
        );

    DashboardWidget =
        CreateDefaultSubobject<UWidgetComponent>(
            TEXT("DashboardWidget")
        );

    DashboardWidget->SetupAttachment(SceneRoot);
    DashboardWidget->SetRelativeLocation(
        FVector(18.0f, 0.0f, 0.0f)
    );
    DashboardWidget->SetWidgetSpace(EWidgetSpace::World);
    DashboardWidget->SetDrawSize(FVector2D(1920.0f, 1080.0f));
    DashboardWidget->SetPivot(FVector2D(0.5f, 0.5f));
    DashboardWidget->SetWidgetClass(
        UStudioPulseDashboardWidget::StaticClass()
    );
    DashboardWidget->SetTwoSided(true);

    ScreenGlow =
        CreateDefaultSubobject<URectLightComponent>(
            TEXT("ScreenGlow")
        );

    ScreenGlow->SetupAttachment(SceneRoot);
    ScreenGlow->SetRelativeLocation(
        FVector(170.0f, 0.0f, 0.0f)
    );
    ScreenGlow->SetRelativeRotation(
        FRotator(0.0f, 180.0f, 0.0f)
    );
    ScreenGlow->SetIntensity(1450.0f);
    ScreenGlow->SetSourceWidth(1600.0f);
    ScreenGlow->SetSourceHeight(850.0f);
    ScreenGlow->SetAttenuationRadius(2200.0f);
    ScreenGlow->SetLightColor(FColor(25, 210, 255));
}


void AStudioPulseDisplayActor::BeginPlay()
{
    Super::BeginPlay();

    if (!DashboardWidget)
    {
        return;
    }

    UStudioPulseDashboardWidget* DashboardInstance =
        CreateWidget<UStudioPulseDashboardWidget>(
            GetWorld(),
            UStudioPulseDashboardWidget::StaticClass()
        );

    if (DashboardInstance)
    {
        DashboardWidget->SetWidget(DashboardInstance);
        DashboardWidget->RequestRedraw();
    }
}
