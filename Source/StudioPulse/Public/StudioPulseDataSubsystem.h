#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StudioPulseLiveData.h"
#include "StudioPulseDataSubsystem.generated.h"

class IWebSocket;


UENUM(BlueprintType)
enum class EStudioPulsePreviewMode : uint8
{
    Automatic UMETA(DisplayName = "Automatic"),
    Live UMETA(DisplayName = "Live"),
    Demo UMETA(DisplayName = "Demo"),
    Warning UMETA(DisplayName = "Warning"),
    Offline UMETA(DisplayName = "Offline")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FStudioPulseLiveDataUpdated,
    FStudioPulseLiveData,
    Data
);

UCLASS()
class STUDIOPULSE_API UStudioPulseDataSubsystem
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(
        FSubsystemCollectionBase& Collection
    ) override;

    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "StudioPulse|Data")
    FStudioPulseLiveDataUpdated OnLiveDataUpdated;

    UFUNCTION(BlueprintPure, Category = "StudioPulse|Data")
    const FStudioPulseLiveData& GetCurrentData() const
    {
        return CurrentData;
    }

    UFUNCTION(BlueprintCallable, Category = "StudioPulse|Data")
    void RequestSnapshot();

    UFUNCTION(BlueprintCallable, Category = "StudioPulse|Data")
    void ConnectWebSocket();

    UFUNCTION(BlueprintCallable, Category = "StudioPulse|Data")
    void DisconnectWebSocket();

    UFUNCTION(BlueprintPure, Category = "StudioPulse|Data")
    bool IsSocketConnected() const
    {
        return bSocketConnected;
    }

    UFUNCTION(BlueprintCallable, Category = "StudioPulse|Data")
    void SetPreviewMode(
        EStudioPulsePreviewMode NewMode
    );

    UFUNCTION(BlueprintPure, Category = "StudioPulse|Data")
    EStudioPulsePreviewMode GetPreviewMode() const
    {
        return PreviewMode;
    }

    static bool ParseLiveDataJson(
        const FString& JsonString,
        FStudioPulseLiveData& OutData
    );

protected:
    UPROPERTY(EditAnywhere, Category = "StudioPulse|Data")
    FString SnapshotUrl =
        TEXT("http://127.0.0.1:8090/snapshot");

    UPROPERTY(EditAnywhere, Category = "StudioPulse|Data")
    FString WebSocketUrl =
        TEXT("ws://127.0.0.1:8091");

    UPROPERTY(
        EditAnywhere,
        Category = "StudioPulse|Data",
        meta = (ClampMin = "0.2")
    )
    float DemoUpdateInterval = 1.0f;

private:
    void UpdateDemoData();
    void HandleSocketConnected();
    void HandleSocketConnectionError(const FString& Error);
    void HandleSocketClosed(
        int32 StatusCode,
        const FString& Reason,
        bool bWasClean
    );
    void HandleSocketMessage(const FString& Message);
    void ScheduleReconnect();
    void ReconnectSocket();
    void ApplyPreviewMode();
    void AppendActivityValue(float NewValue);
    void PublishData(const FStudioPulseLiveData& NewData);

    UPROPERTY(Transient)
    FStudioPulseLiveData CurrentData;

    UPROPERTY(Transient)
    EStudioPulsePreviewMode PreviewMode =
        EStudioPulsePreviewMode::Automatic;

    TSharedPtr<IWebSocket> WebSocket;

    bool bSocketConnected = false;
    bool bReconnectPending = false;
    int32 ReconnectAttempt = 0;
    float DemoPhase = 0.0f;

    FTimerHandle DemoTimerHandle;
    FTimerHandle SnapshotTimerHandle;
    FTimerHandle ReconnectTimerHandle;
};