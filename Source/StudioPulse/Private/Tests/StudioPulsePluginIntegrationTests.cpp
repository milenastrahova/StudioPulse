#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "StudioPulsePluginBridgeActor.h"
#include "VirtualStudioConnectionComponent.h"
#include "VirtualStudioCoreSettings.h"

namespace
{
constexpr EAutomationTestFlags IntegrationTestFlags =
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulsePluginBridgeConstructionTest,
    "StudioPulse.PluginIntegration.BridgeConstruction",
    IntegrationTestFlags
)

bool FStudioPulsePluginBridgeConstructionTest::
    RunTest(
        const FString& Parameters
    )
{
    const AStudioPulsePluginBridgeActor* BridgeCDO =
        GetDefault<
            AStudioPulsePluginBridgeActor
        >();

    TestNotNull(
        TEXT("Bridge class default object"),
        BridgeCDO
    );

    if (!BridgeCDO)
    {
        return false;
    }

    TestNotNull(
        TEXT("Plugin connection component"),
        BridgeCDO->GetConnectionComponent()
    );

    TestFalse(
        TEXT("Host bridge does not Tick"),
        BridgeCDO->
            PrimaryActorTick.bCanEverTick
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulsePluginBridgeReflectionTest,
    "StudioPulse.PluginIntegration.BlueprintSurface",
    IntegrationTestFlags
)

bool FStudioPulsePluginBridgeReflectionTest::
    RunTest(
        const FString& Parameters
    )
{
    UClass* BridgeClass =
        AStudioPulsePluginBridgeActor::
            StaticClass();

    TestNotNull(
        TEXT("ConnectPlugin is reflected"),
        BridgeClass->FindFunctionByName(
            TEXT("ConnectPlugin")
        )
    );

    TestNotNull(
        TEXT("DisconnectPlugin is reflected"),
        BridgeClass->FindFunctionByName(
            TEXT("DisconnectPlugin")
        )
    );

    TestNotNull(
        TEXT("RequestPluginSnapshot is reflected"),
        BridgeClass->FindFunctionByName(
            TEXT("RequestPluginSnapshot")
        )
    );

    TestNotNull(
        TEXT("SendPluginMessage is reflected"),
        BridgeClass->FindFunctionByName(
            TEXT("SendPluginMessage")
        )
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulsePluginSettingsIntegrationTest,
    "StudioPulse.PluginIntegration.SettingsAccess",
    IntegrationTestFlags
)

bool FStudioPulsePluginSettingsIntegrationTest::
    RunTest(
        const FString& Parameters
    )
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<
            UVirtualStudioCoreSettings
        >();

    TestNotNull(
        TEXT("Host project can access plugin settings"),
        Settings
    );

    if (!Settings)
    {
        return false;
    }

    TestTrue(
        TEXT("Configured HTTP endpoint is available"),
        !Settings->SnapshotUrl.IsEmpty()
    );

    TestTrue(
        TEXT("Configured WebSocket endpoint is available"),
        !Settings->WebSocketUrl.IsEmpty()
    );

    return true;
}

#endif