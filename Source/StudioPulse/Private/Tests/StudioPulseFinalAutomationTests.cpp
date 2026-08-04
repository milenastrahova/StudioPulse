#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "StudioPulseDataSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulseValidJsonTest,
    "StudioPulse.Data.ValidJson",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FStudioPulseValidJsonTest::RunTest(
    const FString& Parameters
)
{
    const FString Json =
        TEXT(
            "{"
            "\"streamName\":\"Aurora\","
            "\"status\":\"LIVE\","
            "\"activePlayers\":1800,"
            "\"activeTables\":36,"
            "\"betVolume\":990000,"
            "\"latencyMs\":27.5,"
            "\"uptimePercent\":99.99,"
            "\"alerts\":0,"
            "\"activityHistory\":[10,20,30,40]"
            "}"
        );

    FStudioPulseLiveData Data;

    const bool bParsed =
        UStudioPulseDataSubsystem::ParseLiveDataJson(
            Json,
            Data
        );

    TestTrue(TEXT("Valid JSON parses"), bParsed);
    TestEqual(TEXT("Players"), Data.ActivePlayers, 1800);
    TestEqual(TEXT("Tables"), Data.ActiveTables, 36);
    TestEqual(
        TEXT("History count"),
        Data.ActivityHistory.Num(),
        4
    );

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulseInvalidJsonTest,
    "StudioPulse.Data.InvalidJson",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FStudioPulseInvalidJsonTest::RunTest(
    const FString& Parameters
)
{
    FStudioPulseLiveData Data;

    const bool bParsed =
        UStudioPulseDataSubsystem::ParseLiveDataJson(
            TEXT("{invalid-json"),
            Data
        );

    TestFalse(TEXT("Invalid JSON is rejected"), bParsed);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulsePartialJsonTest,
    "StudioPulse.Data.PartialJsonDefaults",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FStudioPulsePartialJsonTest::RunTest(
    const FString& Parameters
)
{
    FStudioPulseLiveData Data;

    const bool bParsed =
        UStudioPulseDataSubsystem::ParseLiveDataJson(
            TEXT("{\"activePlayers\":777}"),
            Data
        );

    TestTrue(TEXT("Partial JSON parses"), bParsed);
    TestEqual(TEXT("Players update"), Data.ActivePlayers, 777);
    TestEqual(
        TEXT("Default table count remains"),
        Data.ActiveTables,
        42
    );

    return true;
}

#endif
