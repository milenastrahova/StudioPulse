#include "VirtualStudioTelemetryParser.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool TryReadNumber(
    const TSharedPtr<FJsonObject>& Object,
    const TArray<FString>& CandidateNames,
    double& OutValue
)
{
    for (const FString& Name : CandidateNames)
    {
        if (Object->TryGetNumberField(
            Name,
            OutValue
        ))
        {
            return true;
        }
    }

    return false;
}

bool TryReadString(
    const TSharedPtr<FJsonObject>& Object,
    const TArray<FString>& CandidateNames,
    FString& OutValue
)
{
    for (const FString& Name : CandidateNames)
    {
        if (Object->TryGetStringField(
            Name,
            OutValue
        ))
        {
            return true;
        }
    }

    return false;
}

bool HasScheme(
    const FString& Endpoint,
    const TArray<FString>& Schemes
)
{
    FString Trimmed = Endpoint;
    Trimmed.TrimStartAndEndInline();

    for (const FString& Scheme : Schemes)
    {
        if (Trimmed.StartsWith(
            Scheme,
            ESearchCase::IgnoreCase
        ))
        {
            return true;
        }
    }

    return false;
}
}

bool FVirtualStudioTelemetryParser::ParseJson(
    const FString& JsonText,
    FVirtualStudioTelemetryFrame& OutFrame,
    FString& OutError
)
{
    OutFrame = FVirtualStudioTelemetryFrame();
    OutError.Reset();

    TSharedPtr<FJsonObject> JsonObject;

    const TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(JsonText);

    if (!FJsonSerializer::Deserialize(
            Reader,
            JsonObject
        ) ||
        !JsonObject.IsValid())
    {
        OutError =
            TEXT("Payload is not a valid JSON object.");

        return false;
    }

    double OperatorCount = 0.0;
    double LatencyMs = 0.0;
    double UptimeSeconds = 0.0;
    double AlertCount = 0.0;
    FString Status;

    if (!TryReadNumber(
            JsonObject,
            {
                TEXT("operator_count"),
                TEXT("operators")
            },
            OperatorCount
        ))
    {
        OutError =
            TEXT("Missing numeric operator_count field.");

        return false;
    }

    if (!TryReadNumber(
            JsonObject,
            {
                TEXT("latency_ms"),
                TEXT("latency")
            },
            LatencyMs
        ))
    {
        OutError =
            TEXT("Missing numeric latency_ms field.");

        return false;
    }

    if (!TryReadNumber(
            JsonObject,
            {
                TEXT("uptime_seconds"),
                TEXT("uptime")
            },
            UptimeSeconds
        ))
    {
        OutError =
            TEXT("Missing numeric uptime_seconds field.");

        return false;
    }

    if (!TryReadNumber(
            JsonObject,
            {
                TEXT("alert_count"),
                TEXT("alerts")
            },
            AlertCount
        ))
    {
        OutError =
            TEXT("Missing numeric alert_count field.");

        return false;
    }

    if (!TryReadString(
            JsonObject,
            { TEXT("status") },
            Status
        ) ||
        Status.IsEmpty())
    {
        OutError =
            TEXT("Missing non-empty status field.");

        return false;
    }

    FString TimestampUtc;

    TryReadString(
        JsonObject,
        {
            TEXT("timestamp_utc"),
            TEXT("timestamp")
        },
        TimestampUtc
    );

    OutFrame.OperatorCount =
        FMath::Max(
            0,
            FMath::RoundToInt(OperatorCount)
        );

    OutFrame.LatencyMs =
        FMath::Max(
            0.0f,
            static_cast<float>(LatencyMs)
        );

    OutFrame.UptimeSeconds =
        FMath::Max(
            0.0f,
            static_cast<float>(UptimeSeconds)
        );

    OutFrame.AlertCount =
        FMath::Max(
            0,
            FMath::RoundToInt(AlertCount)
        );

    OutFrame.Status = Status;
    OutFrame.Source = TEXT("Network");
    OutFrame.TimestampUtc =
        TimestampUtc.IsEmpty()
            ? FDateTime::UtcNow().ToIso8601()
            : TimestampUtc;

    if (!OutFrame.IsValid())
    {
        OutError =
            TEXT("Parsed telemetry frame is invalid.");

        return false;
    }

    return true;
}

bool FVirtualStudioTelemetryParser::
    IsSupportedHttpEndpoint(
        const FString& Endpoint
    )
{
    return HasScheme(
        Endpoint,
        {
            TEXT("http://"),
            TEXT("https://")
        }
    );
}

bool FVirtualStudioTelemetryParser::
    IsSupportedWebSocketEndpoint(
        const FString& Endpoint
    )
{
    return HasScheme(
        Endpoint,
        {
            TEXT("ws://"),
            TEXT("wss://")
        }
    );
}
