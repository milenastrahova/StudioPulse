#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VirtualStudioCoreTypes.h"
#include "StudioPulsePluginBridgeActor.generated.h"

class UVirtualStudioConnectionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FStudioPulsePluginTelemetryReceived,
    FVirtualStudioTelemetryFrame,
    Frame
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FStudioPulsePluginStateChanged,
    EVirtualStudioConnectionState,
    NewState,
    FString,
    Reason
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FStudioPulsePluginError,
    FString,
    Message
);

/**
 * Host-project integration boundary between StudioPulse and the reusable
 * VirtualStudioCore plugin.
 *
 * The bridge deliberately owns presentation-facing events while the plugin
 * remains responsible for transports, reconnect logic, parsing and fallback.
 */
UCLASS(BlueprintType, Blueprintable)
class STUDIOPULSE_API AStudioPulsePluginBridgeActor
    : public AActor
{
    GENERATED_BODY()

public:
    AStudioPulsePluginBridgeActor();

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "StudioPulse|Virtual Studio Core"
    )
    TObjectPtr<UVirtualStudioConnectionComponent>
        ConnectionComponent;

    UPROPERTY(
        BlueprintAssignable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    FStudioPulsePluginTelemetryReceived
        OnPluginTelemetryReceived;

    UPROPERTY(
        BlueprintAssignable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    FStudioPulsePluginStateChanged
        OnPluginStateChanged;

    UPROPERTY(
        BlueprintAssignable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    FStudioPulsePluginError
        OnPluginError;

    UFUNCTION(
        BlueprintCallable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    void ConnectPlugin();

    UFUNCTION(
        BlueprintCallable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    void DisconnectPlugin();

    UFUNCTION(
        BlueprintCallable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    void RequestPluginSnapshot();

    UFUNCTION(
        BlueprintCallable,
        Category = "StudioPulse|Virtual Studio Core"
    )
    bool SendPluginMessage(const FString& Message);

    UFUNCTION(
        BlueprintPure,
        Category = "StudioPulse|Virtual Studio Core"
    )
    UVirtualStudioConnectionComponent*
        GetConnectionComponent() const;

    UFUNCTION(
        BlueprintPure,
        Category = "StudioPulse|Virtual Studio Core"
    )
    FVirtualStudioTelemetryFrame
        GetLatestPluginFrame() const;

    UFUNCTION(
        BlueprintPure,
        Category = "StudioPulse|Virtual Studio Core"
    )
    EVirtualStudioConnectionState
        GetPluginConnectionState() const;

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

private:
    UFUNCTION()
    void HandlePluginTelemetry(
        FVirtualStudioTelemetryFrame Frame
    );

    UFUNCTION()
    void HandlePluginStateChanged(
        EVirtualStudioConnectionState NewState,
        FString Reason
    );

    UFUNCTION()
    void HandlePluginError(
        FString Message
    );

    UPROPERTY(Transient)
    FVirtualStudioTelemetryFrame
        LatestPluginFrame;
};