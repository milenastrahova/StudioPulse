#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "StudioPulseDataSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStudioPulseJsonParsingTest,
    "StudioPulse.Data.JsonParsing",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FStudioPulseJsonParsingTest::RunTest(
    const FString& Parameters
)
{
    const FString Json =
        TEXT(
            "{"
            "\"streamName\":\"Test Table\","
            "\"status\":\"LIVE\","
            "\"activePlayers\":1200,"
            "\"activeTables\":24,"
            "\"betVolume\":875000,"
            "\"latencyMs\":31.5,"
            "\"uptimePercent\":99.98,"
            "\"alerts\":0,"
            "\"activityHistory\":[20,40,60]"
            "}"
        );

    FStudioPulseLiveData Data;

    const bool bParsed =
        UStudioPulseDataSubsystem::ParseLiveDataJson(
            Json,
            Data
        );

    TestTrue(TEXT("JSON parses"), bParsed);
    TestEqual(TEXT("Players"), Data.ActivePlayers, 1200);
    TestEqual(TEXT("Tables"), Data.ActiveTables, 24);
    TestEqual(
        TEXT("History count"),
        Data.ActivityHistory.Num(),
        3
    );

    return true;
}

#endif
