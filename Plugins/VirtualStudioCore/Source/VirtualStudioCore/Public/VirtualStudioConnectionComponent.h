#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "IWebSocket.h"
#include "VirtualStudioCoreTypes.h"
#include "VirtualStudioConnectionComponent.generated.h"

UCLASS(
    ClassGroup = (VirtualStudio),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent)
)
class VIRTUALSTUDIOCORE_API UVirtualStudioConnectionComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UVirtualStudioConnectionComponent();

    UPROPERTY(
        BlueprintAssignable,
        Category = "Virtual Studio|Events"
    )
    FVirtualStudioTelemetryUpdated
        OnTelemetryUpdated;

    UPROPERTY(
        BlueprintAssignable,
        Category = "Virtual Studio|Events"
    )
    FVirtualStudioConnectionStateChanged
        OnConnectionStateChanged;

    UPROPERTY(
        BlueprintAssignable,
        Category = "Virtual Studio|Events"
    )
    FVirtualStudioTransportError
        OnTransportError;

    UFUNCTION(
        BlueprintCallable,
        Category = "Virtual Studio|Connection"
    )
    void Connect();

    UFUNCTION(
        BlueprintCallable,
        Category = "Virtual Studio|Connection"
    )
    void Disconnect();

    UFUNCTION(
        BlueprintCallable,
        Category = "Virtual Studio|HTTP"
    )
    void RequestSnapshot();

    UFUNCTION(
        BlueprintCallable,
        Category = "Virtual Studio|WebSocket"
    )
    bool SendTextMessage(const FString& Message);

    UFUNCTION(
        BlueprintPure,
        Category = "Virtual Studio|Connection"
    )
    EVirtualStudioConnectionState
        GetConnectionState() const;

    UFUNCTION(
        BlueprintPure,
        Category = "Virtual Studio|Telemetry"
    )
    FVirtualStudioTelemetryFrame
        GetLastFrame() const;

    UFUNCTION(
        BlueprintPure,
        Category = "Virtual Studio|Connection"
    )
    int32 GetReconnectAttemptCount() const;

    UFUNCTION(
        BlueprintPure,
        Category = "Virtual Studio|Connection"
    )
    bool IsConnected() const;

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

private:
    void StartWebSocket();

    void HandleWebSocketConnected();

    void HandleWebSocketConnectionError(
        const FString& Error
    );

    void HandleWebSocketClosed(
        int32 StatusCode,
        const FString& Reason,
        bool bWasClean
    );

    void HandleWebSocketMessage(
        const FString& Message
    );

    void HandleSnapshotResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bSucceeded
    );

    void HandleTransportFailure(
        const FString& Reason
    );

    void ScheduleReconnect(
        const FString& Reason
    );

    void PerformReconnect();

    void StartSimulatedFallback(
        const FString& Reason
    );

    void StopSimulatedFallback();

    void GenerateSimulatedFrame();

    void PublishFrame(
        const FVirtualStudioTelemetryFrame& Frame
    );

    void SetConnectionState(
        EVirtualStudioConnectionState NewState,
        const FString& Reason
    );

    void ClearRuntimeTimers();

    TSharedPtr<IWebSocket> WebSocket;

    FHttpRequestPtr ActiveSnapshotRequest;

    FTimerHandle ReconnectTimerHandle;
    FTimerHandle SimulationTimerHandle;

    UPROPERTY(Transient)
    EVirtualStudioConnectionState
        ConnectionState =
            EVirtualStudioConnectionState::
                Disconnected;

    UPROPERTY(Transient)
    FVirtualStudioTelemetryFrame LastFrame;

    int32 ReconnectAttemptCount = 0;
    int32 SimulationSequence = 0;

    bool bManualDisconnect = false;
    bool bReconnectScheduled = false;
};
