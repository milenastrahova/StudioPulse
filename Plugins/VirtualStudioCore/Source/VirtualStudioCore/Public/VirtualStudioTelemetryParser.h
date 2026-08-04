#pragma once

#include "CoreMinimal.h"
#include "VirtualStudioCoreTypes.h"

class VIRTUALSTUDIOCORE_API FVirtualStudioTelemetryParser
{
public:
    static bool ParseJson(
        const FString& JsonText,
        FVirtualStudioTelemetryFrame& OutFrame,
        FString& OutError
    );

    static bool IsSupportedHttpEndpoint(
        const FString& Endpoint
    );

    static bool IsSupportedWebSocketEndpoint(
        const FString& Endpoint
    );
};
