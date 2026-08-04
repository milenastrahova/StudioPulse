#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StudioPulseDisplayActor.generated.h"

class URectLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class STUDIOPULSE_API AStudioPulseDisplayActor
    : public AActor
{
    GENERATED_BODY()

public:
    AStudioPulseDisplayActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UWidgetComponent> DashboardWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> ScreenBack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> FrameTop;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> FrameBottom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> FrameLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> FrameRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> AccentLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> AccentRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<UStaticMeshComponent> HeaderGlow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StudioPulse")
    TObjectPtr<URectLightComponent> ScreenGlow;
};
