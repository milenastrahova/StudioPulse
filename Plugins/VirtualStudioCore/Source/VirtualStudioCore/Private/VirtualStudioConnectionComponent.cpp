#include "VirtualStudioConnectionComponent.h"

#include "Engine/World.h"
#include "HttpModule.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"
#include "VirtualStudioCoreSettings.h"
#include "VirtualStudioTelemetryParser.h"
#include "WebSocketsModule.h"

UVirtualStudioConnectionComponent::
    UVirtualStudioConnectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVirtualStudioConnectionComponent::BeginPlay()
{
    Super::BeginPlay();

    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (Settings && Settings->bAutoConnect)
    {
        Connect();
    }
}

void UVirtualStudioConnectionComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    Disconnect();

    Super::EndPlay(EndPlayReason);
}

void UVirtualStudioConnectionComponent::Connect()
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (!Settings)
    {
        SetConnectionState(
            EVirtualStudioConnectionState::Error,
            TEXT("Virtual Studio settings are unavailable.")
        );

        return;
    }

    bManualDisconnect = false;
    bReconnectScheduled = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            ReconnectTimerHandle
        );
    }

    const bool bKeepFallbackVisible =
        ConnectionState ==
            EVirtualStudioConnectionState::
                SimulatedFallback &&
        Settings->bEnableSimulatedFallback;

    if (!bKeepFallbackVisible)
    {
        SetConnectionState(
            EVirtualStudioConnectionState::Connecting,
            TEXT("Starting configured transports.")
        );
    }

    bool bStartedAnyTransport = false;

    if (Settings->bEnableWebSocket &&
        FVirtualStudioTelemetryParser::
            IsSupportedWebSocketEndpoint(
                Settings->WebSocketUrl
            ))
    {
        StartWebSocket();
        bStartedAnyTransport = true;
    }

    if (Settings->bEnableHttpSnapshot &&
        FVirtualStudioTelemetryParser::
            IsSupportedHttpEndpoint(
                Settings->SnapshotUrl
            ))
    {
        RequestSnapshot();
        bStartedAnyTransport = true;
    }

    if (!bStartedAnyTransport)
    {
        HandleTransportFailure(
            TEXT(
                "No valid HTTP or WebSocket "
                "transport is enabled."
            )
        );
    }
}

void UVirtualStudioConnectionComponent::Disconnect()
{
    bManualDisconnect = true;
    bReconnectScheduled = false;

    ClearRuntimeTimers();

    if (ActiveSnapshotRequest.IsValid())
    {
        ActiveSnapshotRequest->CancelRequest();
        ActiveSnapshotRequest.Reset();
    }

    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().RemoveAll(this);
        WebSocket->OnConnectionError().RemoveAll(this);
        WebSocket->OnClosed().RemoveAll(this);
        WebSocket->OnMessage().RemoveAll(this);

        if (WebSocket->IsConnected())
        {
            WebSocket->Close();
        }

        WebSocket.Reset();
    }

    SetConnectionState(
        EVirtualStudioConnectionState::Disconnected,
        TEXT("Disconnected by owner.")
    );
}

void UVirtualStudioConnectionComponent::
    StartWebSocket()
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (!Settings)
    {
        return;
    }

    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().RemoveAll(this);
        WebSocket->OnConnectionError().RemoveAll(this);
        WebSocket->OnClosed().RemoveAll(this);
        WebSocket->OnMessage().RemoveAll(this);

        if (WebSocket->IsConnected())
        {
            WebSocket->Close();
        }

        WebSocket.Reset();
    }

    FWebSocketsModule& WebSocketsModule =
        FModuleManager::LoadModuleChecked<
            FWebSocketsModule
        >(TEXT("WebSockets"));

    WebSocket =
        WebSocketsModule.CreateWebSocket(
            Settings->WebSocketUrl
        );

    WebSocket->OnConnected().AddUObject(
        this,
        &UVirtualStudioConnectionComponent::
            HandleWebSocketConnected
    );

    WebSocket->OnConnectionError().AddUObject(
        this,
        &UVirtualStudioConnectionComponent::
            HandleWebSocketConnectionError
    );

    WebSocket->OnClosed().AddUObject(
        this,
        &UVirtualStudioConnectionComponent::
            HandleWebSocketClosed
    );

    WebSocket->OnMessage().AddUObject(
        this,
        &UVirtualStudioConnectionComponent::
            HandleWebSocketMessage
    );

    WebSocket->Connect();
}

void UVirtualStudioConnectionComponent::
    HandleWebSocketConnected()
{
    ReconnectAttemptCount = 0;
    bReconnectScheduled = false;

    StopSimulatedFallback();

    SetConnectionState(
        EVirtualStudioConnectionState::Connected,
        TEXT("WebSocket connected.")
    );
}

void UVirtualStudioConnectionComponent::
    HandleWebSocketConnectionError(
        const FString& Error
    )
{
    HandleTransportFailure(
        FString::Printf(
            TEXT("WebSocket error: %s"),
            *Error
        )
    );
}

void UVirtualStudioConnectionComponent::
    HandleWebSocketClosed(
        const int32 StatusCode,
        const FString& Reason,
        const bool bWasClean
    )
{
    if (bManualDisconnect)
    {
        return;
    }

    HandleTransportFailure(
        FString::Printf(
            TEXT(
                "WebSocket closed (%d, clean=%s): %s"
            ),
            StatusCode,
            bWasClean ? TEXT("true") : TEXT("false"),
            *Reason
        )
    );
}

void UVirtualStudioConnectionComponent::
    HandleWebSocketMessage(
        const FString& Message
    )
{
    FVirtualStudioTelemetryFrame Frame;
    FString Error;

    if (!FVirtualStudioTelemetryParser::ParseJson(
            Message,
            Frame,
            Error
        ))
    {
        OnTransportError.Broadcast(
            FString::Printf(
                TEXT(
                    "WebSocket payload rejected: %s"
                ),
                *Error
            )
        );

        return;
    }

    Frame.Source = TEXT("WebSocket");

    ReconnectAttemptCount = 0;
    bReconnectScheduled = false;

    StopSimulatedFallback();

    SetConnectionState(
        EVirtualStudioConnectionState::Connected,
        TEXT("Valid WebSocket telemetry received.")
    );

    PublishFrame(Frame);
}

void UVirtualStudioConnectionComponent::
    RequestSnapshot()
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (!Settings ||
        !Settings->bEnableHttpSnapshot ||
        !FVirtualStudioTelemetryParser::
            IsSupportedHttpEndpoint(
                Settings->SnapshotUrl
            ))
    {
        return;
    }

    if (ActiveSnapshotRequest.IsValid())
    {
        ActiveSnapshotRequest->CancelRequest();
        ActiveSnapshotRequest.Reset();
    }

    ActiveSnapshotRequest =
        FHttpModule::Get().CreateRequest();

    ActiveSnapshotRequest->SetURL(
        Settings->SnapshotUrl
    );

    ActiveSnapshotRequest->SetVerb(
        TEXT("GET")
    );

    ActiveSnapshotRequest->SetHeader(
        TEXT("Accept"),
        TEXT("application/json")
    );

    ActiveSnapshotRequest->
        OnProcessRequestComplete().BindUObject(
            this,
            &UVirtualStudioConnectionComponent::
                HandleSnapshotResponse
        );

    if (!ActiveSnapshotRequest->ProcessRequest())
    {
        ActiveSnapshotRequest.Reset();

        HandleTransportFailure(
            TEXT(
                "HTTP snapshot request could not "
                "be started."
            )
        );
    }
}

void UVirtualStudioConnectionComponent::
    HandleSnapshotResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        const bool bSucceeded
    )
{
    ActiveSnapshotRequest.Reset();

    if (!bSucceeded ||
        !Response.IsValid())
    {
        HandleTransportFailure(
            TEXT("HTTP snapshot request failed.")
        );

        return;
    }

    const int32 ResponseCode =
        Response->GetResponseCode();

    if (ResponseCode < 200 ||
        ResponseCode >= 300)
    {
        HandleTransportFailure(
            FString::Printf(
                TEXT(
                    "HTTP snapshot returned status %d."
                ),
                ResponseCode
            )
        );

        return;
    }

    FVirtualStudioTelemetryFrame Frame;
    FString Error;

    if (!FVirtualStudioTelemetryParser::ParseJson(
            Response->GetContentAsString(),
            Frame,
            Error
        ))
    {
        HandleTransportFailure(
            FString::Printf(
                TEXT(
                    "HTTP snapshot payload rejected: %s"
                ),
                *Error
            )
        );

        return;
    }

    Frame.Source = TEXT("HTTP");

    ReconnectAttemptCount = 0;
    bReconnectScheduled = false;

    StopSimulatedFallback();

    SetConnectionState(
        EVirtualStudioConnectionState::Connected,
        TEXT("Valid HTTP snapshot received.")
    );

    PublishFrame(Frame);
}

bool UVirtualStudioConnectionComponent::
    SendTextMessage(
        const FString& Message
    )
{
    if (!WebSocket.IsValid() ||
        !WebSocket->IsConnected() ||
        Message.IsEmpty())
    {
        return false;
    }

    WebSocket->Send(Message);

    return true;
}

EVirtualStudioConnectionState
UVirtualStudioConnectionComponent::
    GetConnectionState() const
{
    return ConnectionState;
}

FVirtualStudioTelemetryFrame
UVirtualStudioConnectionComponent::
    GetLastFrame() const
{
    return LastFrame;
}

int32 UVirtualStudioConnectionComponent::
    GetReconnectAttemptCount() const
{
    return ReconnectAttemptCount;
}

bool UVirtualStudioConnectionComponent::
    IsConnected() const
{
    return ConnectionState ==
        EVirtualStudioConnectionState::Connected;
}

void UVirtualStudioConnectionComponent::
    HandleTransportFailure(
        const FString& Reason
    )
{
    OnTransportError.Broadcast(Reason);

    if (bManualDisconnect)
    {
        return;
    }

    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (Settings &&
        Settings->bEnableSimulatedFallback)
    {
        StartSimulatedFallback(Reason);
    }
    else
    {
        SetConnectionState(
            EVirtualStudioConnectionState::Reconnecting,
            Reason
        );
    }

    ScheduleReconnect(Reason);
}

void UVirtualStudioConnectionComponent::
    ScheduleReconnect(
        const FString& Reason
    )
{
    if (bManualDisconnect ||
        bReconnectScheduled)
    {
        return;
    }

    UWorld* World = GetWorld();

    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    if (!World || !Settings)
    {
        SetConnectionState(
            EVirtualStudioConnectionState::Error,
            TEXT(
                "Reconnect scheduling requires "
                "a valid World and settings object."
            )
        );

        return;
    }

    const float Delay =
        Settings->ReconnectPolicy.GetDelaySeconds(
            ReconnectAttemptCount
        );

    ++ReconnectAttemptCount;
    bReconnectScheduled = true;

    World->GetTimerManager().SetTimer(
        ReconnectTimerHandle,
        this,
        &UVirtualStudioConnectionComponent::
            PerformReconnect,
        Delay,
        false
    );
}

void UVirtualStudioConnectionComponent::
    PerformReconnect()
{
    bReconnectScheduled = false;

    Connect();
}

void UVirtualStudioConnectionComponent::
    StartSimulatedFallback(
        const FString& Reason
    )
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    UWorld* World = GetWorld();

    if (!Settings || !World)
    {
        return;
    }

    SetConnectionState(
        EVirtualStudioConnectionState::
            SimulatedFallback,
        Reason
    );

    if (!World->GetTimerManager().IsTimerActive(
            SimulationTimerHandle
        ))
    {
        GenerateSimulatedFrame();

        World->GetTimerManager().SetTimer(
            SimulationTimerHandle,
            this,
            &UVirtualStudioConnectionComponent::
                GenerateSimulatedFrame,
            FMath::Max(
                0.1f,
                Settings->SimulationIntervalSeconds
            ),
            true
        );
    }
}

void UVirtualStudioConnectionComponent::
    StopSimulatedFallback()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            SimulationTimerHandle
        );
    }
}

void UVirtualStudioConnectionComponent::
    GenerateSimulatedFrame()
{
    ++SimulationSequence;

    const float Phase =
        static_cast<float>(SimulationSequence);

    FVirtualStudioTelemetryFrame Frame;

    Frame.OperatorCount =
        8 + (SimulationSequence % 5);

    Frame.LatencyMs =
        42.0f +
        18.0f *
        FMath::Sin(Phase * 0.35f);

    Frame.UptimeSeconds =
        static_cast<float>(SimulationSequence);

    Frame.AlertCount =
        (SimulationSequence % 9 == 0)
            ? 1
            : 0;

    Frame.Status =
        Frame.AlertCount > 0
            ? TEXT("Warning")
            : TEXT("Operational");

    Frame.Source =
        TEXT("SimulatedFallback");

    Frame.TimestampUtc =
        FDateTime::UtcNow().ToIso8601();

    PublishFrame(Frame);
}

void UVirtualStudioConnectionComponent::PublishFrame(
    const FVirtualStudioTelemetryFrame& Frame
)
{
    LastFrame = Frame;

    OnTelemetryUpdated.Broadcast(LastFrame);
}

void UVirtualStudioConnectionComponent::
    SetConnectionState(
        const EVirtualStudioConnectionState NewState,
        const FString& Reason
    )
{
    if (ConnectionState == NewState)
    {
        return;
    }

    ConnectionState = NewState;

    OnConnectionStateChanged.Broadcast(
        ConnectionState,
        Reason
    );
}

void UVirtualStudioConnectionComponent::
    ClearRuntimeTimers()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            ReconnectTimerHandle
        );

        World->GetTimerManager().ClearTimer(
            SimulationTimerHandle
        );
    }
}
