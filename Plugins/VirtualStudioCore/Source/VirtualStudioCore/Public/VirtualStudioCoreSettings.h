#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VirtualStudioCoreTypes.h"
#include "VirtualStudioCoreSettings.generated.h"

UCLASS(
    Config = Game,
    DefaultConfig,
    meta = (DisplayName = "Virtual Studio Core")
)
class VIRTUALSTUDIOCORE_API UVirtualStudioCoreSettings
    : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UVirtualStudioCoreSettings();

    virtual FName GetCategoryName() const override;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Endpoints"
    )
    FString SnapshotUrl;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Endpoints"
    )
    FString WebSocketUrl;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Startup"
    )
    bool bAutoConnect = true;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Transports"
    )
    bool bEnableHttpSnapshot = true;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Transports"
    )
    bool bEnableWebSocket = true;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Fallback"
    )
    bool bEnableSimulatedFallback = true;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Fallback",
        meta = (ClampMin = "0.1", UIMin = "0.1")
    )
    float SimulationIntervalSeconds = 1.0f;

    UPROPERTY(
        Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Reconnect"
    )
    FVirtualStudioReconnectPolicy ReconnectPolicy;
};
