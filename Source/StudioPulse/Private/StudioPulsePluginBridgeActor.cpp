#include "StudioPulsePluginBridgeActor.h"

#include "VirtualStudioConnectionComponent.h"

AStudioPulsePluginBridgeActor::
    AStudioPulsePluginBridgeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ConnectionComponent =
        CreateDefaultSubobject<
            UVirtualStudioConnectionComponent
        >(
            TEXT("VirtualStudioConnection")
        );
}

void AStudioPulsePluginBridgeActor::BeginPlay()
{
    Super::BeginPlay();

    if (!ConnectionComponent)
    {
        return;
    }

    ConnectionComponent->
        OnTelemetryUpdated.AddUniqueDynamic(
            this,
            &AStudioPulsePluginBridgeActor::
                HandlePluginTelemetry
        );

    ConnectionComponent->
        OnConnectionStateChanged.AddUniqueDynamic(
            this,
            &AStudioPulsePluginBridgeActor::
                HandlePluginStateChanged
        );

    ConnectionComponent->
        OnTransportError.AddUniqueDynamic(
            this,
            &AStudioPulsePluginBridgeActor::
                HandlePluginError
        );

    LatestPluginFrame =
        ConnectionComponent->GetLastFrame();

    if (ConnectionComponent->
            GetConnectionState() ==
        EVirtualStudioConnectionState::
            Disconnected)
    {
        ConnectionComponent->Connect();
    }
}

void AStudioPulsePluginBridgeActor::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    if (ConnectionComponent)
    {
        ConnectionComponent->
            OnTelemetryUpdated.RemoveDynamic(
                this,
                &AStudioPulsePluginBridgeActor::
                    HandlePluginTelemetry
            );

        ConnectionComponent->
            OnConnectionStateChanged.RemoveDynamic(
                this,
                &AStudioPulsePluginBridgeActor::
                    HandlePluginStateChanged
            );

        ConnectionComponent->
            OnTransportError.RemoveDynamic(
                this,
                &AStudioPulsePluginBridgeActor::
                    HandlePluginError
            );
    }

    Super::EndPlay(EndPlayReason);
}

void AStudioPulsePluginBridgeActor::
    ConnectPlugin()
{
    if (ConnectionComponent)
    {
        ConnectionComponent->Connect();
    }
}

void AStudioPulsePluginBridgeActor::
    DisconnectPlugin()
{
    if (ConnectionComponent)
    {
        ConnectionComponent->Disconnect();
    }
}

void AStudioPulsePluginBridgeActor::
    RequestPluginSnapshot()
{
    if (ConnectionComponent)
    {
        ConnectionComponent->RequestSnapshot();
    }
}

bool AStudioPulsePluginBridgeActor::
    SendPluginMessage(
        const FString& Message
    )
{
    return ConnectionComponent &&
        ConnectionComponent->
            SendTextMessage(Message);
}

UVirtualStudioConnectionComponent*
AStudioPulsePluginBridgeActor::
    GetConnectionComponent() const
{
    return ConnectionComponent;
}

FVirtualStudioTelemetryFrame
AStudioPulsePluginBridgeActor::
    GetLatestPluginFrame() const
{
    return LatestPluginFrame;
}

EVirtualStudioConnectionState
AStudioPulsePluginBridgeActor::
    GetPluginConnectionState() const
{
    return ConnectionComponent
        ? ConnectionComponent->
            GetConnectionState()
        : EVirtualStudioConnectionState::
            Disconnected;
}

void AStudioPulsePluginBridgeActor::
    HandlePluginTelemetry(
        FVirtualStudioTelemetryFrame Frame
    )
{
    LatestPluginFrame = Frame;

    OnPluginTelemetryReceived.Broadcast(
        LatestPluginFrame
    );
}

void AStudioPulsePluginBridgeActor::
    HandlePluginStateChanged(
        const EVirtualStudioConnectionState NewState,
        FString Reason
    )
{
    OnPluginStateChanged.Broadcast(
        NewState,
        Reason
    );
}

void AStudioPulsePluginBridgeActor::
    HandlePluginError(
        FString Message
    )
{
    OnPluginError.Broadcast(Message);
}