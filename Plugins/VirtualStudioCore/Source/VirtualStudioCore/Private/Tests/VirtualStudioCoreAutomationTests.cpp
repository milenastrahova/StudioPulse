#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "VirtualStudioConnectionComponent.h"
#include "VirtualStudioCoreSettings.h"
#include "VirtualStudioCoreTypes.h"
#include "VirtualStudioTelemetryParser.h"

namespace
{
constexpr EAutomationTestFlags TestFlags =
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioReconnectPolicyTest,
    "VirtualStudioCore.Reconnect.ExponentialBackoff",
    TestFlags
)

bool FVirtualStudioReconnectPolicyTest::RunTest(
    const FString& Parameters
)
{
    FVirtualStudioReconnectPolicy Policy;

    Policy.BaseDelaySeconds = 1.0f;
    Policy.MaximumDelaySeconds = 8.0f;
    Policy.Multiplier = 2.0f;

    TestEqual(
        TEXT("Attempt 0"),
        Policy.GetDelaySeconds(0),
        1.0f
    );

    TestEqual(
        TEXT("Attempt 1"),
        Policy.GetDelaySeconds(1),
        2.0f
    );

    TestEqual(
        TEXT("Attempt 2"),
        Policy.GetDelaySeconds(2),
        4.0f
    );

    TestEqual(
        TEXT("Maximum clamp"),
        Policy.GetDelaySeconds(10),
        8.0f
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioValidPayloadTest,
    "VirtualStudioCore.Parser.ValidPayload",
    TestFlags
)

bool FVirtualStudioValidPayloadTest::RunTest(
    const FString& Parameters
)
{
    const FString Json = TEXT(
        "{"
        "\"operator_count\":12,"
        "\"latency_ms\":48.5,"
        "\"uptime_seconds\":3600,"
        "\"alert_count\":1,"
        "\"status\":\"Warning\","
        "\"timestamp_utc\":\"2026-08-04T12:00:00Z\""
        "}"
    );

    FVirtualStudioTelemetryFrame Frame;
    FString Error;

    TestTrue(
        TEXT("Valid payload parses"),
        FVirtualStudioTelemetryParser::ParseJson(
            Json,
            Frame,
            Error
        )
    );

    TestEqual(
        TEXT("Operator count"),
        Frame.OperatorCount,
        12
    );

    TestEqual(
        TEXT("Alert count"),
        Frame.AlertCount,
        1
    );

    TestEqual(
        TEXT("Status"),
        Frame.Status,
        FString(TEXT("Warning"))
    );

    TestTrue(
        TEXT("Frame is valid"),
        Frame.IsValid()
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioAliasPayloadTest,
    "VirtualStudioCore.Parser.AliasPayload",
    TestFlags
)

bool FVirtualStudioAliasPayloadTest::RunTest(
    const FString& Parameters
)
{
    const FString Json = TEXT(
        "{"
        "\"operators\":4,"
        "\"latency\":20,"
        "\"uptime\":120,"
        "\"alerts\":0,"
        "\"status\":\"Operational\""
        "}"
    );

    FVirtualStudioTelemetryFrame Frame;
    FString Error;

    TestTrue(
        TEXT("Alias payload parses"),
        FVirtualStudioTelemetryParser::ParseJson(
            Json,
            Frame,
            Error
        )
    );

    TestEqual(
        TEXT("Alias operator count"),
        Frame.OperatorCount,
        4
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioInvalidPayloadTest,
    "VirtualStudioCore.Parser.InvalidPayload",
    TestFlags
)

bool FVirtualStudioInvalidPayloadTest::RunTest(
    const FString& Parameters
)
{
    FVirtualStudioTelemetryFrame Frame;
    FString Error;

    TestFalse(
        TEXT("Invalid JSON is rejected"),
        FVirtualStudioTelemetryParser::ParseJson(
            TEXT("{invalid"),
            Frame,
            Error
        )
    );

    TestTrue(
        TEXT("Error message is populated"),
        !Error.IsEmpty()
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioEndpointValidationTest,
    "VirtualStudioCore.Configuration.EndpointValidation",
    TestFlags
)

bool FVirtualStudioEndpointValidationTest::RunTest(
    const FString& Parameters
)
{
    TestTrue(
        TEXT("HTTP endpoint"),
        FVirtualStudioTelemetryParser::
            IsSupportedHttpEndpoint(
                TEXT("http://127.0.0.1:8090/snapshot")
            )
    );

    TestTrue(
        TEXT("Secure WebSocket endpoint"),
        FVirtualStudioTelemetryParser::
            IsSupportedWebSocketEndpoint(
                TEXT("wss://example.com/live")
            )
    );

    TestFalse(
        TEXT("FTP is rejected"),
        FVirtualStudioTelemetryParser::
            IsSupportedHttpEndpoint(
                TEXT("ftp://example.com")
            )
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioSettingsDefaultsTest,
    "VirtualStudioCore.Configuration.DefaultSettings",
    TestFlags
)

bool FVirtualStudioSettingsDefaultsTest::RunTest(
    const FString& Parameters
)
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    TestNotNull(
        TEXT("Settings object"),
        Settings
    );

    if (!Settings)
    {
        return false;
    }

    TestTrue(
        TEXT("HTTP enabled"),
        Settings->bEnableHttpSnapshot
    );

    TestTrue(
        TEXT("WebSocket enabled"),
        Settings->bEnableWebSocket
    );

    TestTrue(
        TEXT("Fallback enabled"),
        Settings->bEnableSimulatedFallback
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioComponentNoTickTest,
    "VirtualStudioCore.Performance.ComponentDoesNotTick",
    TestFlags
)

bool FVirtualStudioComponentNoTickTest::RunTest(
    const FString& Parameters
)
{
    UVirtualStudioConnectionComponent* Component =
        NewObject<UVirtualStudioConnectionComponent>();

    TestNotNull(
        TEXT("Component created"),
        Component
    );

    if (!Component)
    {
        return false;
    }

    TestFalse(
        TEXT("Per-frame Tick is disabled"),
        Component->PrimaryComponentTick.bCanEverTick
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVirtualStudioBlueprintSurfaceTest,
    "VirtualStudioCore.API.BlueprintSurface",
    TestFlags
)

bool FVirtualStudioBlueprintSurfaceTest::RunTest(
    const FString& Parameters
)
{
    UClass* ComponentClass =
        UVirtualStudioConnectionComponent::
            StaticClass();

    TestNotNull(
        TEXT("Connect is reflected"),
        ComponentClass->FindFunctionByName(
            TEXT("Connect")
        )
    );

    TestNotNull(
        TEXT("Disconnect is reflected"),
        ComponentClass->FindFunctionByName(
            TEXT("Disconnect")
        )
    );

    TestNotNull(
        TEXT("RequestSnapshot is reflected"),
        ComponentClass->FindFunctionByName(
            TEXT("RequestSnapshot")
        )
    );

    TestNotNull(
        TEXT("SendTextMessage is reflected"),
        ComponentClass->FindFunctionByName(
            TEXT("SendTextMessage")
        )
    );

    return true;
}

#endif
