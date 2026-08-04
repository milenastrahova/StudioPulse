#pragma once

#include "CoreMinimal.h"
#include "StudioPulseLiveData.generated.h"

USTRUCT(BlueprintType)
struct FStudioPulseLiveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    FString StreamName = TEXT("AURORA LIVE TABLE");

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    FString Status = TEXT("DEMO");

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    FString DataSource = TEXT("SIMULATED FEED");

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    int32 ActivePlayers = 2486;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    int32 ActiveTables = 42;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    int64 BetVolume = 1842500;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    float LatencyMs = 28.0f;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    float UptimePercent = 99.97f;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    int32 Alerts = 0;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    TArray<float> ActivityHistory;

    UPROPERTY(BlueprintReadOnly, Category = "StudioPulse")
    FString LastUpdateIso;
};
