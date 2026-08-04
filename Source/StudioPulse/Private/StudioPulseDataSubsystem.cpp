#include "StudioPulseDataSubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "IWebSocket.h"
#include "Json.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"
#include "WebSocketsModule.h"

void UStudioPulseDataSubsystem::Initialize(
    FSubsystemCollectionBase& Collection
)
{
    Super::Initialize(Collection);

    CurrentData.ActivityHistory.Reserve(24);

    for (int32 Index = 0; Index < 24; ++Index)
    {
        CurrentData.ActivityHistory.Add(
            45.0f + FMath::Sin(Index * 0.45f) * 18.0f
        );
    }

    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DemoTimerHandle,
            this,
            &UStudioPulseDataSubsystem::UpdateDemoData,
            DemoUpdateInterval,
            true,
            DemoUpdateInterval
        );

        World->GetTimerManager().SetTimer(
            SnapshotTimerHandle,
            this,
            &UStudioPulseDataSubsystem::RequestSnapshot,
            10.0f,
            true,
            0.15f
        );
    }

    ConnectWebSocket();
}

void UStudioPulseDataSubsystem::Deinitialize()
{
    DisconnectWebSocket();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DemoTimerHandle);
        World->GetTimerManager().ClearTimer(SnapshotTimerHandle);
        World->GetTimerManager().ClearTimer(ReconnectTimerHandle);
    }

    Super::Deinitialize();
}

void UStudioPulseDataSubsystem::RequestSnapshot()
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        return;
    }

    if (SnapshotUrl.IsEmpty())
    {
        return;
    }

    TSharedRef<IHttpRequest> Request =
        FHttpModule::Get().CreateRequest();

    Request->SetURL(SnapshotUrl);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(
        TEXT("Accept"),
        TEXT("application/json")
    );

    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](
            FHttpRequestPtr RequestPtr,
            FHttpResponsePtr Response,
            bool bSucceeded
        )
        {
            if (PreviewMode !=
                EStudioPulsePreviewMode::Automatic)
            {
                return;
            }

            if (!bSucceeded ||
                !Response.IsValid() ||
                Response->GetResponseCode() < 200 ||
                Response->GetResponseCode() >= 300)
            {
                return;
            }

            FStudioPulseLiveData ParsedData;

            if (ParseLiveDataJson(
                Response->GetContentAsString(),
                ParsedData
            ))
            {
                ParsedData.DataSource =
                    TEXT("HTTP SNAPSHOT");

                PublishData(ParsedData);
            }
        }
    );

    Request->ProcessRequest();
}

void UStudioPulseDataSubsystem::ConnectWebSocket()
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        return;
    }

    if (WebSocketUrl.IsEmpty() ||
        bSocketConnected ||
        WebSocket.IsValid())
    {
        return;
    }

    FWebSocketsModule& WebSocketsModule =
        FModuleManager::LoadModuleChecked<
            FWebSocketsModule
        >(TEXT("WebSockets"));

    WebSocket =
        WebSocketsModule.CreateWebSocket(
            WebSocketUrl
        );

    WebSocket->OnConnected().AddUObject(
        this,
        &UStudioPulseDataSubsystem::HandleSocketConnected
    );

    WebSocket->OnConnectionError().AddUObject(
        this,
        &UStudioPulseDataSubsystem::
        HandleSocketConnectionError
    );

    WebSocket->OnClosed().AddUObject(
        this,
        &UStudioPulseDataSubsystem::HandleSocketClosed
    );

    WebSocket->OnMessage().AddUObject(
        this,
        &UStudioPulseDataSubsystem::HandleSocketMessage
    );

    WebSocket->Connect();
}

void UStudioPulseDataSubsystem::DisconnectWebSocket()
{
    bSocketConnected = false;

    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().RemoveAll(this);
        WebSocket->OnConnectionError().RemoveAll(this);
        WebSocket->OnClosed().RemoveAll(this);
        WebSocket->OnMessage().RemoveAll(this);

        WebSocket->Close();
        WebSocket.Reset();
    }
}

void UStudioPulseDataSubsystem::HandleSocketConnected()
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        DisconnectWebSocket();
        return;
    }

    bSocketConnected = true;
    bReconnectPending = false;
    ReconnectAttempt = 0;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            ReconnectTimerHandle
        );
    }

    CurrentData.Status = TEXT("LIVE");
    CurrentData.DataSource = TEXT("WEBSOCKET");
    CurrentData.Alerts = 0;
    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);
}

void UStudioPulseDataSubsystem::
HandleSocketConnectionError(
    const FString& Error
)
{
    bSocketConnected = false;
    WebSocket.Reset();

    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        bReconnectPending = false;
        return;
    }

    bReconnectPending = true;
    ++ReconnectAttempt;

    CurrentData.Status = TEXT("DEMO");
    CurrentData.DataSource =
        FString::Printf(
            TEXT("SIMULATED FALLBACK / RETRY %d"),
            ReconnectAttempt
        );
    CurrentData.Alerts = 0;
    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);
    ScheduleReconnect();
}

void UStudioPulseDataSubsystem::HandleSocketClosed(
    int32 StatusCode,
    const FString& Reason,
    bool bWasClean
)
{
    bSocketConnected = false;
    WebSocket.Reset();

    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        bReconnectPending = false;
        return;
    }

    bReconnectPending = true;
    ++ReconnectAttempt;

    CurrentData.Status = TEXT("DEMO");
    CurrentData.DataSource =
        FString::Printf(
            TEXT("SIMULATED FALLBACK / RETRY %d"),
            ReconnectAttempt
        );
    CurrentData.Alerts = 0;
    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);
    ScheduleReconnect();
}

void UStudioPulseDataSubsystem::HandleSocketMessage(
    const FString& Message
)
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        return;
    }

    FStudioPulseLiveData ParsedData;

    if (!ParseLiveDataJson(
        Message,
        ParsedData
    ))
    {
        return;
    }

    bSocketConnected = true;
    bReconnectPending = false;
    ReconnectAttempt = 0;

    ParsedData.Status = TEXT("LIVE");
    ParsedData.DataSource = TEXT("WEBSOCKET");
    ParsedData.Alerts = 0;

    PublishData(ParsedData);
}

void UStudioPulseDataSubsystem::ScheduleReconnect()
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        return;
    }

    if (bSocketConnected)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        if (!World->GetTimerManager().IsTimerActive(
            ReconnectTimerHandle
        ))
        {
            World->GetTimerManager().SetTimer(
                ReconnectTimerHandle,
                this,
                &UStudioPulseDataSubsystem::ReconnectSocket,
                5.0f,
                false
            );
        }
    }
}

void UStudioPulseDataSubsystem::ReconnectSocket()
{
    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        return;
    }

    DisconnectWebSocket();
    ConnectWebSocket();
}

void UStudioPulseDataSubsystem::UpdateDemoData()
{
    DemoPhase += 0.22f;

    if (PreviewMode !=
        EStudioPulsePreviewMode::Automatic)
    {
        ApplyPreviewMode();

        CurrentData.BetVolume +=
            FMath::RandRange(900, 6200);

        CurrentData.LastUpdateIso =
            FDateTime::UtcNow().ToIso8601();

        PublishData(CurrentData);
        return;
    }

    if (bSocketConnected)
    {
        return;
    }

    CurrentData.StreamName =
        TEXT("AURORA LIVE TABLE");

    CurrentData.Status = TEXT("DEMO");

    CurrentData.DataSource =
        bReconnectPending
        ? FString::Printf(
            TEXT("SIMULATED FALLBACK / RETRY %d"),
            FMath::Max(ReconnectAttempt, 1)
        )
        : TEXT("SIMULATED FALLBACK");

    CurrentData.ActivePlayers =
        FMath::RoundToInt(
            2450.0f +
            FMath::Sin(DemoPhase) * 240.0f +
            FMath::FRandRange(-40.0f, 40.0f)
        );

    CurrentData.ActiveTables =
        42 +
        FMath::RoundToInt(
            FMath::Sin(DemoPhase * 0.65f) * 4.0f
        );

    CurrentData.BetVolume +=
        FMath::RandRange(1800, 9200);

    CurrentData.LatencyMs =
        FMath::Clamp(
            31.0f +
            FMath::Sin(DemoPhase * 1.4f) * 20.0f +
            FMath::FRandRange(-7.0f, 7.0f),
            14.0f,
            92.0f
        );

    CurrentData.UptimePercent = 99.97f;
    CurrentData.Alerts = 0;

    AppendActivityValue(
        FMath::Clamp(
            58.0f +
            FMath::Sin(DemoPhase * 0.9f) * 24.0f +
            FMath::FRandRange(-6.0f, 6.0f),
            8.0f,
            100.0f
        )
    );

    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);
}

void UStudioPulseDataSubsystem::SetPreviewMode(
    EStudioPulsePreviewMode NewMode
)
{
    PreviewMode = NewMode;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            ReconnectTimerHandle
        );
    }

    if (PreviewMode ==
        EStudioPulsePreviewMode::Automatic)
    {
        bReconnectPending = false;
        ReconnectAttempt = 0;

        CurrentData.Status =
            bSocketConnected
            ? TEXT("LIVE")
            : TEXT("DEMO");

        CurrentData.DataSource =
            bSocketConnected
            ? TEXT("WEBSOCKET")
            : TEXT("SIMULATED FALLBACK");

        CurrentData.Alerts = 0;

        if (!bSocketConnected)
        {
            ConnectWebSocket();
            RequestSnapshot();
        }
    }
    else
    {
        DisconnectWebSocket();

        bReconnectPending = false;
        ReconnectAttempt = 0;

        ApplyPreviewMode();
    }

    CurrentData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    PublishData(CurrentData);
}

void UStudioPulseDataSubsystem::ApplyPreviewMode()
{
    CurrentData.StreamName =
        TEXT("AURORA LIVE TABLE");

    switch (PreviewMode)
    {
    case EStudioPulsePreviewMode::Live:
        CurrentData.Status = TEXT("LIVE");
        CurrentData.DataSource =
            TEXT("MANUAL OPERATOR MODE");
        CurrentData.ActivePlayers =
            FMath::RoundToInt(
                2680.0f +
                FMath::Sin(DemoPhase) * 145.0f
            );
        CurrentData.ActiveTables =
            46 +
            FMath::RoundToInt(
                FMath::Sin(DemoPhase * 0.55f) * 2.0f
            );
        CurrentData.LatencyMs =
            24.0f +
            FMath::Sin(DemoPhase * 1.3f) * 7.0f;
        CurrentData.UptimePercent = 99.99f;
        CurrentData.Alerts = 0;
        AppendActivityValue(
            72.0f +
            FMath::Sin(DemoPhase * 0.85f) * 18.0f
        );
        break;

    case EStudioPulsePreviewMode::Demo:
        CurrentData.Status = TEXT("DEMO");
        CurrentData.DataSource =
            TEXT("SIMULATED DEMO FEED");
        CurrentData.ActivePlayers =
            FMath::RoundToInt(
                2450.0f +
                FMath::Sin(DemoPhase) * 240.0f +
                FMath::FRandRange(-40.0f, 40.0f)
            );
        CurrentData.ActiveTables =
            42 +
            FMath::RoundToInt(
                FMath::Sin(DemoPhase * 0.65f) * 4.0f
            );
        CurrentData.LatencyMs =
            FMath::Clamp(
                31.0f +
                FMath::Sin(DemoPhase * 1.4f) * 20.0f +
                FMath::FRandRange(-7.0f, 7.0f),
                14.0f,
                92.0f
            );
        CurrentData.UptimePercent = 99.97f;
        CurrentData.Alerts = 0;
        AppendActivityValue(
            FMath::Clamp(
                58.0f +
                FMath::Sin(DemoPhase * 0.9f) * 24.0f +
                FMath::FRandRange(-6.0f, 6.0f),
                8.0f,
                100.0f
            )
        );
        break;

    case EStudioPulsePreviewMode::Warning:
        CurrentData.Status = TEXT("WARNING");
        CurrentData.DataSource =
            TEXT("LATENCY INCIDENT");
        CurrentData.ActivePlayers =
            FMath::RoundToInt(
                2310.0f +
                FMath::Sin(DemoPhase) * 90.0f
            );
        CurrentData.ActiveTables = 39;
        CurrentData.LatencyMs =
            124.0f +
            FMath::Sin(DemoPhase * 1.6f) * 14.0f;
        CurrentData.UptimePercent = 99.93f;
        CurrentData.Alerts = 3;
        AppendActivityValue(
            42.0f +
            FMath::Sin(DemoPhase * 1.1f) * 17.0f
        );
        break;

    case EStudioPulsePreviewMode::Offline:
        CurrentData.Status = TEXT("OFFLINE");
        CurrentData.DataSource =
            TEXT("CONNECTION LOST");
        CurrentData.ActivePlayers = 0;
        CurrentData.ActiveTables = 0;
        CurrentData.LatencyMs = 0.0f;
        CurrentData.UptimePercent = 99.88f;
        CurrentData.Alerts = 5;
        AppendActivityValue(
            7.0f +
            FMath::Abs(
                FMath::Sin(DemoPhase)
            ) * 6.0f
        );
        break;

    case EStudioPulsePreviewMode::Automatic:
    default:
        break;
    }
}

void UStudioPulseDataSubsystem::AppendActivityValue(
    float NewValue
)
{
    if (CurrentData.ActivityHistory.Num() >= 24)
    {
        CurrentData.ActivityHistory.RemoveAt(0);
    }

    CurrentData.ActivityHistory.Add(
        FMath::Clamp(
            NewValue,
            4.0f,
            100.0f
        )
    );
}

void UStudioPulseDataSubsystem::PublishData(
    const FStudioPulseLiveData& NewData
)
{
    CurrentData = NewData;

    if (CurrentData.ActivityHistory.Num() == 0)
    {
        for (int32 Index = 0; Index < 24; ++Index)
        {
            CurrentData.ActivityHistory.Add(50.0f);
        }
    }

    OnLiveDataUpdated.Broadcast(CurrentData);
}

bool UStudioPulseDataSubsystem::ParseLiveDataJson(
    const FString& JsonString,
    FStudioPulseLiveData& OutData
)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(
        Reader,
        RootObject
    ) ||
        !RootObject.IsValid())
    {
        return false;
    }

    RootObject->TryGetStringField(
        TEXT("streamName"),
        OutData.StreamName
    );

    RootObject->TryGetStringField(
        TEXT("status"),
        OutData.Status
    );

    double NumberValue = 0.0;

    if (RootObject->TryGetNumberField(
        TEXT("activePlayers"),
        NumberValue
    ))
    {
        OutData.ActivePlayers =
            FMath::RoundToInt(NumberValue);
    }

    if (RootObject->TryGetNumberField(
        TEXT("activeTables"),
        NumberValue
    ))
    {
        OutData.ActiveTables =
            FMath::RoundToInt(NumberValue);
    }

    if (RootObject->TryGetNumberField(
        TEXT("betVolume"),
        NumberValue
    ))
    {
        OutData.BetVolume =
            static_cast<int64>(NumberValue);
    }

    if (RootObject->TryGetNumberField(
        TEXT("latencyMs"),
        NumberValue
    ))
    {
        OutData.LatencyMs =
            static_cast<float>(NumberValue);
    }

    if (RootObject->TryGetNumberField(
        TEXT("uptimePercent"),
        NumberValue
    ))
    {
        OutData.UptimePercent =
            static_cast<float>(NumberValue);
    }

    if (RootObject->TryGetNumberField(
        TEXT("alerts"),
        NumberValue
    ))
    {
        OutData.Alerts =
            FMath::RoundToInt(NumberValue);
    }

    const TArray<TSharedPtr<FJsonValue>>*
        HistoryArray = nullptr;

    if (RootObject->TryGetArrayField(
        TEXT("activityHistory"),
        HistoryArray
    ) &&
        HistoryArray)
    {
        OutData.ActivityHistory.Reset();

        for (const TSharedPtr<FJsonValue>& Value
            : *HistoryArray)
        {
            OutData.ActivityHistory.Add(
                static_cast<float>(
                    Value->AsNumber()
                )
            );
        }
    }

    OutData.LastUpdateIso =
        FDateTime::UtcNow().ToIso8601();

    return true;
}