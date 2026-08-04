#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StudioPulseLiveData.h"
#include "StudioPulseDashboardWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UCanvasPanelSlot;
class UProgressBar;
class UTextBlock;
class UStudioPulseDataSubsystem;

UCLASS()
class STUDIOPULSE_API UStudioPulseDashboardWidget
    : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void BuildDashboard();

    UFUNCTION()
    void HandleLiveDataUpdated(
        FStudioPulseLiveData Data
    );

    UTextBlock* CreateText(
        const FString& Text,
        int32 FontSize,
        const FLinearColor& Color
    );

    UBorder* CreateCard(
        const FLinearColor& BackgroundColor
    );

    UCanvasPanelSlot* AddCanvasWidget(
        UCanvasPanel* Canvas,
        UWidget* Widget,
        const FVector2D& Position,
        const FVector2D& Size
    );

    void UpdateActivityChart(
        const TArray<float>& Values
    );

    UPROPERTY(Transient)
    TObjectPtr<UStudioPulseDataSubsystem> DataSubsystem;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StatusText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SourceText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PlayersValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TablesValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> VolumeValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> LatencyValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> UptimeValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> AlertValue;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> UpdatedValue;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> StatusPill;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> LatencyBar;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UCanvasPanelSlot>> ActivityBarSlots;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> SystemStateValues;
};
