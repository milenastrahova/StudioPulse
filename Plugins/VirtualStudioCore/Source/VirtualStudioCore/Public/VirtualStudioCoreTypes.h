#pragma once

#include "CoreMinimal.h"
#include "VirtualStudioCoreTypes.generated.h"

UENUM(BlueprintType)
enum class EVirtualStudioConnectionState : uint8
{
    Disconnected UMETA(DisplayName = "Disconnected"),
    Connecting UMETA(DisplayName = "Connecting"),
    Connected UMETA(DisplayName = "Connected"),
    Reconnecting UMETA(DisplayName = "Reconnecting"),
    SimulatedFallback UMETA(DisplayName = "Simulated Fallback"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct VIRTUALSTUDIOCORE_API FVirtualStudioTelemetryFrame
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    int32 OperatorCount = 0;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    float LatencyMs = 0.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    float UptimeSeconds = 0.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    int32 AlertCount = 0;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    FString Status = TEXT("Unknown");

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    FString Source = TEXT("Unknown");

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Virtual Studio"
    )
    FString TimestampUtc;

    bool IsValid() const;
};

USTRUCT(BlueprintType)
struct VIRTUALSTUDIOCORE_API FVirtualStudioReconnectPolicy
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Reconnect",
        meta = (ClampMin = "0.1", UIMin = "0.1")
    )
    float BaseDelaySeconds = 1.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Reconnect",
        meta = (ClampMin = "0.1", UIMin = "0.1")
    )
    float MaximumDelaySeconds = 30.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Reconnect",
        meta = (ClampMin = "1.0", UIMin = "1.0")
    )
    float Multiplier = 2.0f;

    float GetDelaySeconds(int32 AttemptIndex) const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FVirtualStudioTelemetryUpdated,
    FVirtualStudioTelemetryFrame,
    Frame
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVirtualStudioConnectionStateChanged,
    EVirtualStudioConnectionState,
    NewState,
    FString,
    Reason
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FVirtualStudioTransportError,
    FString,
    Message
);