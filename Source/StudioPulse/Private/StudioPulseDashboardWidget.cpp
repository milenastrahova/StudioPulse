#include "StudioPulseDashboardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "StudioPulseDataSubsystem.h"

namespace StudioPulseStyle
{
    const FLinearColor Background(0.010f, 0.016f, 0.028f, 1.0f);
    const FLinearColor Panel(0.026f, 0.043f, 0.068f, 0.96f);
    const FLinearColor PanelAlt(0.035f, 0.060f, 0.092f, 0.96f);
    const FLinearColor Cyan(0.10f, 0.93f, 1.0f, 1.0f);
    const FLinearColor Magenta(1.0f, 0.13f, 0.67f, 1.0f);
    const FLinearColor Gold(1.0f, 0.73f, 0.23f, 1.0f);
    const FLinearColor White(0.93f, 0.97f, 1.0f, 1.0f);
    const FLinearColor Muted(0.45f, 0.58f, 0.70f, 1.0f);
    const FLinearColor Success(0.17f, 1.0f, 0.53f, 1.0f);
    const FLinearColor Warning(1.0f, 0.30f, 0.20f, 1.0f);
}

TSharedRef<SWidget>
UStudioPulseDashboardWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(
            this,
            TEXT("StudioPulseWidgetTree"),
            RF_Transient
        );
    }

    if (!WidgetTree->RootWidget)
    {
        BuildDashboard();
    }

    return Super::RebuildWidget();
}

void UStudioPulseDashboardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        DataSubsystem =
            GameInstance->GetSubsystem<UStudioPulseDataSubsystem>();

        if (DataSubsystem)
        {
            DataSubsystem->OnLiveDataUpdated.AddDynamic(
                this,
                &UStudioPulseDashboardWidget::HandleLiveDataUpdated
            );

            HandleLiveDataUpdated(
                DataSubsystem->GetCurrentData()
            );
        }
    }
}

void UStudioPulseDashboardWidget::NativeDestruct()
{
    if (DataSubsystem)
    {
        DataSubsystem->OnLiveDataUpdated.RemoveDynamic(
            this,
            &UStudioPulseDashboardWidget::HandleLiveDataUpdated
        );
    }

    Super::NativeDestruct();
}

UTextBlock* UStudioPulseDashboardWidget::CreateText(
    const FString& Text,
    int32 FontSize,
    const FLinearColor& Color
)
{
    UTextBlock* TextBlock =
        WidgetTree->ConstructWidget<UTextBlock>();

    TextBlock->SetText(FText::FromString(Text));
    TextBlock->SetColorAndOpacity(FSlateColor(Color));

    FSlateFontInfo FontInfo = TextBlock->GetFont();
    FontInfo.Size = FontSize;
    TextBlock->SetFont(FontInfo);

    TextBlock->SetAutoWrapText(false);
    TextBlock->SetShadowOffset(FVector2D(1.0f, 2.0f));
    TextBlock->SetShadowColorAndOpacity(
        FLinearColor(0.0f, 0.0f, 0.0f, 0.45f)
    );

    return TextBlock;
}

UBorder* UStudioPulseDashboardWidget::CreateCard(
    const FLinearColor& BackgroundColor
)
{
    UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
    Border->SetBrushColor(BackgroundColor);
    Border->SetPadding(FMargin(26.0f, 20.0f));
    return Border;
}

UCanvasPanelSlot*
UStudioPulseDashboardWidget::AddCanvasWidget(
    UCanvasPanel* Canvas,
    UWidget* Widget,
    const FVector2D& Position,
    const FVector2D& Size
)
{
    UCanvasPanelSlot* CanvasSlot =
        Canvas->AddChildToCanvas(Widget);

    CanvasSlot->SetPosition(Position);
    CanvasSlot->SetSize(Size);
    CanvasSlot->SetAutoSize(false);

    return CanvasSlot;
}

void UStudioPulseDashboardWidget::BuildDashboard()
{
    if (!WidgetTree)
    {
        return;
    }

    UCanvasPanel* Root =
        WidgetTree->ConstructWidget<UCanvasPanel>(
            UCanvasPanel::StaticClass(),
            TEXT("StudioPulseRoot")
        );

    WidgetTree->RootWidget = Root;

    UBorder* Background =
        CreateCard(StudioPulseStyle::Background);

    AddCanvasWidget(
        Root,
        Background,
        FVector2D::ZeroVector,
        FVector2D(1920.0f, 1080.0f)
    );

    UCanvasPanel* Content =
        WidgetTree->ConstructWidget<UCanvasPanel>();

    Background->SetContent(Content);

    UBorder* Header =
        CreateCard(StudioPulseStyle::Panel);

    AddCanvasWidget(
        Content,
        Header,
        FVector2D(46.0f, 34.0f),
        FVector2D(1828.0f, 116.0f)
    );

    UHorizontalBox* HeaderBox =
        WidgetTree->ConstructWidget<UHorizontalBox>();

    Header->SetContent(HeaderBox);

    UVerticalBox* TitleStack =
        WidgetTree->ConstructWidget<UVerticalBox>();

    UHorizontalBoxSlot* TitleSlot =
        HeaderBox->AddChildToHorizontalBox(TitleStack);

    TitleSlot->SetSize(
        FSlateChildSize(ESlateSizeRule::Fill)
    );

    TitleStack->AddChildToVerticalBox(
        CreateText(
            TEXT("STUDIOPULSE"),
            42,
            StudioPulseStyle::White
        )
    );

    TitleStack->AddChildToVerticalBox(
        CreateText(
            TEXT("REAL-TIME VIRTUAL STUDIO OPERATIONS WALL"),
            18,
            StudioPulseStyle::Muted
        )
    );

    UVerticalBox* StatusStack =
        WidgetTree->ConstructWidget<UVerticalBox>();

    UHorizontalBoxSlot* StatusStackSlot =
        HeaderBox->AddChildToHorizontalBox(StatusStack);

    StatusStackSlot->SetHorizontalAlignment(HAlign_Right);
    StatusStackSlot->SetVerticalAlignment(VAlign_Center);

    StatusPill = CreateCard(StudioPulseStyle::Success);
    StatusPill->SetPadding(FMargin(28.0f, 8.0f));

    StatusText =
        CreateText(TEXT("LIVE"), 22, FLinearColor::Black);

    StatusText->SetJustification(ETextJustify::Center);
    StatusPill->SetContent(StatusText);
    StatusStack->AddChildToVerticalBox(StatusPill);

    SourceText =
        CreateText(
            TEXT("SIMULATED FEED"),
            14,
            StudioPulseStyle::Muted
        );

    SourceText->SetJustification(ETextJustify::Right);
    StatusStack->AddChildToVerticalBox(SourceText);

    const FVector2D CardSize(420.0f, 250.0f);

    const FVector2D CardPositions[] =
    {
        FVector2D(46.0f, 180.0f),
        FVector2D(486.0f, 180.0f),
        FVector2D(926.0f, 180.0f)
    };

    const FString CardLabels[] =
    {
        TEXT("ACTIVE PLAYERS"),
        TEXT("LIVE TABLES"),
        TEXT("BET VOLUME")
    };

    const FLinearColor CardAccents[] =
    {
        StudioPulseStyle::Cyan,
        StudioPulseStyle::Magenta,
        StudioPulseStyle::Gold
    };

    for (int32 Index = 0; Index < 3; ++Index)
    {
        UBorder* MetricCard =
            CreateCard(StudioPulseStyle::PanelAlt);

        AddCanvasWidget(
            Content,
            MetricCard,
            CardPositions[Index],
            CardSize
        );

        UVerticalBox* MetricStack =
            WidgetTree->ConstructWidget<UVerticalBox>();

        MetricCard->SetContent(MetricStack);

        MetricStack->AddChildToVerticalBox(
            CreateText(
                CardLabels[Index],
                16,
                StudioPulseStyle::Muted
            )
        );

        UTextBlock* Value =
            CreateText(
                TEXT("--"),
                56,
                CardAccents[Index]
            );

        MetricStack->AddChildToVerticalBox(Value);

        switch (Index)
        {
        case 0:
            PlayersValue = Value;
            break;

        case 1:
            TablesValue = Value;
            break;

        case 2:
            VolumeValue = Value;
            break;

        default:
            break;
        }

        MetricStack->AddChildToVerticalBox(
            CreateText(
                Index == 2
                    ? TEXT("EUR / CURRENT SESSION")
                    : TEXT("LIVE NETWORK TOTAL"),
                14,
                StudioPulseStyle::Muted
            )
        );
    }

    UBorder* NetworkCard =
        CreateCard(StudioPulseStyle::Panel);

    AddCanvasWidget(
        Content,
        NetworkCard,
        FVector2D(1366.0f, 180.0f),
        FVector2D(508.0f, 250.0f)
    );

    UVerticalBox* NetworkStack =
        WidgetTree->ConstructWidget<UVerticalBox>();

    NetworkCard->SetContent(NetworkStack);

    NetworkStack->AddChildToVerticalBox(
        CreateText(
            TEXT("NETWORK HEALTH"),
            17,
            StudioPulseStyle::White
        )
    );

    LatencyValue =
        CreateText(
            TEXT("-- MS"),
            44,
            StudioPulseStyle::Cyan
        );

    NetworkStack->AddChildToVerticalBox(LatencyValue);

    LatencyBar =
        WidgetTree->ConstructWidget<UProgressBar>();

    LatencyBar->SetFillColorAndOpacity(
        StudioPulseStyle::Cyan
    );
    LatencyBar->SetPercent(0.25f);

    UVerticalBoxSlot* BarSlot =
        NetworkStack->AddChildToVerticalBox(LatencyBar);

    BarSlot->SetPadding(
        FMargin(0.0f, 10.0f, 0.0f, 18.0f)
    );

    UHorizontalBox* NetworkBottom =
        WidgetTree->ConstructWidget<UHorizontalBox>();

    NetworkStack->AddChildToVerticalBox(NetworkBottom);

    UptimeValue =
        CreateText(
            TEXT("UPTIME --"),
            16,
            StudioPulseStyle::Success
        );

    NetworkBottom->AddChildToHorizontalBox(UptimeValue);

    AlertValue =
        CreateText(
            TEXT("ALERTS 0"),
            16,
            StudioPulseStyle::Muted
        );

    UHorizontalBoxSlot* AlertSlot =
        NetworkBottom->AddChildToHorizontalBox(AlertValue);

    AlertSlot->SetHorizontalAlignment(HAlign_Right);
    AlertSlot->SetSize(
        FSlateChildSize(ESlateSizeRule::Fill)
    );

    UBorder* ChartCard =
        CreateCard(StudioPulseStyle::Panel);

    AddCanvasWidget(
        Content,
        ChartCard,
        FVector2D(46.0f, 458.0f),
        FVector2D(1310.0f, 564.0f)
    );

    UCanvasPanel* ChartContent =
        WidgetTree->ConstructWidget<UCanvasPanel>();

    ChartCard->SetContent(ChartContent);

    AddCanvasWidget(
        ChartContent,
        CreateText(
            TEXT("LIVE ACTIVITY"),
            20,
            StudioPulseStyle::White
        ),
        FVector2D(0.0f, 0.0f),
        FVector2D(400.0f, 34.0f)
    );

    AddCanvasWidget(
        ChartContent,
        CreateText(
            TEXT("24-SECOND REAL-TIME THROUGHPUT WINDOW"),
            14,
            StudioPulseStyle::Muted
        ),
        FVector2D(0.0f, 34.0f),
        FVector2D(520.0f, 28.0f)
    );

    UCanvasPanel* ActivityCanvas =
        WidgetTree->ConstructWidget<UCanvasPanel>();

    AddCanvasWidget(
        ChartContent,
        ActivityCanvas,
        FVector2D(0.0f, 86.0f),
        FVector2D(1250.0f, 390.0f)
    );

    ActivityBarSlots.Reset();

    for (int32 Index = 0; Index < 24; ++Index)
    {
        UBorder* Bar =
            CreateCard(
                Index % 5 == 0
                    ? StudioPulseStyle::Magenta
                    : StudioPulseStyle::Cyan
            );

        Bar->SetPadding(FMargin(0.0f));

        const float X =
            Index * 50.5f + 9.0f;

        UCanvasPanelSlot* ActivityCanvasSlot =
            AddCanvasWidget(
                ActivityCanvas,
                Bar,
                FVector2D(X, 290.0f),
                FVector2D(26.0f, 80.0f)
            );

        ActivityBarSlots.Add(ActivityCanvasSlot);
    }

    UBorder* OverviewCard =
        CreateCard(StudioPulseStyle::PanelAlt);

    AddCanvasWidget(
        Content,
        OverviewCard,
        FVector2D(1366.0f, 458.0f),
        FVector2D(508.0f, 564.0f)
    );

    UVerticalBox* OverviewStack =
        WidgetTree->ConstructWidget<UVerticalBox>();

    OverviewCard->SetContent(OverviewStack);

    OverviewStack->AddChildToVerticalBox(
        CreateText(
            TEXT("SYSTEM OVERVIEW"),
            20,
            StudioPulseStyle::White
        )
    );

    const FString OverviewRows[] =
    {
        TEXT("DATA INGEST"),
        TEXT("TABLE ROUTING"),
        TEXT("RENDER PIPELINE"),
        TEXT("LED OUTPUT"),
        TEXT("FAILOVER")
    };

    for (const FString& Row : OverviewRows)
    {
        UHorizontalBox* RowBox =
            WidgetTree->ConstructWidget<UHorizontalBox>();

        UVerticalBoxSlot* RowSlot =
            OverviewStack->AddChildToVerticalBox(RowBox);

        RowSlot->SetPadding(
            FMargin(0.0f, 18.0f, 0.0f, 0.0f)
        );

        RowBox->AddChildToHorizontalBox(
            CreateText(
                Row,
                16,
                StudioPulseStyle::Muted
            )
        );

        UTextBlock* Online =
            CreateText(
                TEXT("ONLINE"),
                16,
                StudioPulseStyle::Success
            );

        SystemStateValues.Add(Online);

        UHorizontalBoxSlot* OnlineSlot =
            RowBox->AddChildToHorizontalBox(Online);

        OnlineSlot->SetSize(
            FSlateChildSize(ESlateSizeRule::Fill)
        );
        OnlineSlot->SetHorizontalAlignment(HAlign_Right);
    }

    UpdatedValue =
        CreateText(
            TEXT("LAST UPDATE --"),
            13,
            StudioPulseStyle::Muted
        );

    UVerticalBoxSlot* UpdatedSlot =
        OverviewStack->AddChildToVerticalBox(UpdatedValue);

    UpdatedSlot->SetPadding(
        FMargin(0.0f, 32.0f, 0.0f, 0.0f)
    );

    AddCanvasWidget(
        Content,
        CreateText(
            TEXT(
                "OPERATOR PREVIEW   "
                "1 LIVE   2 WARNING   "
                "3 OFFLINE   4 DEMO   5 AUTO"
            ),
            14,
            StudioPulseStyle::Muted
        ),
        FVector2D(46.0f, 1032.0f),
        FVector2D(1828.0f, 28.0f)
    );
}

void UStudioPulseDashboardWidget::
HandleLiveDataUpdated(
    FStudioPulseLiveData Data
)
{
    if (StatusText)
    {
        StatusText->SetText(
            FText::FromString(Data.Status)
        );
    }

    if (SourceText)
    {
        SourceText->SetText(
            FText::FromString(Data.DataSource)
        );
    }

    const bool bWarning =
        Data.Status.Equals(
            TEXT("WARNING"),
            ESearchCase::IgnoreCase
        );

    const bool bLive =
        Data.Status.Equals(
            TEXT("LIVE"),
            ESearchCase::IgnoreCase
        );

    const bool bDemo =
        Data.Status.Equals(
            TEXT("DEMO"),
            ESearchCase::IgnoreCase
        );

    const bool bOffline =
        Data.Status.Equals(
            TEXT("OFFLINE"),
            ESearchCase::IgnoreCase
        );

    const bool bReconnecting =
        Data.Status.Equals(
            TEXT("RECONNECTING"),
            ESearchCase::IgnoreCase
        );

    FLinearColor StateColor =
        StudioPulseStyle::Gold;

    if (bOffline)
    {
        StateColor = StudioPulseStyle::Warning;
    }
    else if (bReconnecting)
    {
        StateColor = StudioPulseStyle::Magenta;
    }
    else if (bWarning)
    {
        StateColor = StudioPulseStyle::Gold;
    }
    else if (bDemo)
    {
        StateColor = StudioPulseStyle::Cyan;
    }
    else if (bLive)
    {
        StateColor = StudioPulseStyle::Success;
    }

    if (StatusPill)
    {
        StatusPill->SetBrushColor(StateColor);
    }

    if (PlayersValue)
    {
        PlayersValue->SetText(
            FText::AsNumber(Data.ActivePlayers)
        );
    }

    if (TablesValue)
    {
        TablesValue->SetText(
            FText::AsNumber(Data.ActiveTables)
        );
    }

    if (VolumeValue)
    {
        VolumeValue->SetText(
            FText::AsNumber(Data.BetVolume)
        );
    }

    if (LatencyValue)
    {
        FText LatencyText =
            FText::FromString(
                FString::Printf(
                    TEXT("%.0f MS"),
                    Data.LatencyMs
                )
            );

        if (bOffline)
        {
            LatencyText =
                FText::FromString(TEXT("NO LINK"));
        }
        else if (bReconnecting)
        {
            LatencyText =
                FText::FromString(TEXT("RETRY"));
        }

        LatencyValue->SetText(LatencyText);
        LatencyValue->SetColorAndOpacity(
            FSlateColor(StateColor)
        );
    }

    if (LatencyBar)
    {
        const float LatencyPercent =
            (bOffline || bReconnecting)
            ? 0.0f
            : FMath::Clamp(
                Data.LatencyMs / 150.0f,
                0.0f,
                1.0f
            );

        LatencyBar->SetPercent(LatencyPercent);
        LatencyBar->SetFillColorAndOpacity(
            StateColor
        );
    }

    if (UptimeValue)
    {
        UptimeValue->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("UPTIME %.2f%%"),
                    Data.UptimePercent
                )
            )
        );
    }

    if (AlertValue)
    {
        AlertValue->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("ALERTS %d"),
                    Data.Alerts
                )
            )
        );

        AlertValue->SetColorAndOpacity(
            FSlateColor(
                Data.Alerts > 0
                ? StateColor
                : StudioPulseStyle::Muted
            )
        );
    }

    if (UpdatedValue)
    {
        UpdatedValue->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("LAST UPDATE  %s"),
                    *Data.LastUpdateIso
                )
            )
        );
    }

    if (SystemStateValues.Num() >= 5)
    {
        const FString StatesLive[] =
        {
            TEXT("ONLINE"),
            TEXT("ONLINE"),
            TEXT("ONLINE"),
            TEXT("ONLINE"),
            TEXT("ARMED")
        };

        const FString StatesWarning[] =
        {
            TEXT("DEGRADED"),
            TEXT("ONLINE"),
            TEXT("ONLINE"),
            TEXT("ONLINE"),
            TEXT("ARMED")
        };

        const FString StatesOffline[] =
        {
            TEXT("OFFLINE"),
            TEXT("HOLD"),
            TEXT("ONLINE"),
            TEXT("LOCAL"),
            TEXT("ACTIVE")
        };

        const FString StatesReconnect[] =
        {
            TEXT("RETRYING"),
            TEXT("FALLBACK"),
            TEXT("ONLINE"),
            TEXT("LOCAL"),
            TEXT("ACTIVE")
        };

        const FString StatesDemo[] =
        {
            TEXT("SIMULATED"),
            TEXT("LOCAL"),
            TEXT("ONLINE"),
            TEXT("LOCAL"),
            TEXT("READY")
        };

        for (int32 Index = 0; Index < 5; ++Index)
        {
            UTextBlock* StateValue =
                SystemStateValues[Index];

            if (!StateValue)
            {
                continue;
            }

            FString StateText =
                StatesDemo[Index];

            if (bOffline)
            {
                StateText =
                    StatesOffline[Index];
            }
            else if (bReconnecting)
            {
                StateText =
                    StatesReconnect[Index];
            }
            else if (bWarning)
            {
                StateText =
                    StatesWarning[Index];
            }
            else if (bLive)
            {
                StateText =
                    StatesLive[Index];
            }

            StateValue->SetText(
                FText::FromString(StateText)
            );

            StateValue->SetColorAndOpacity(
                FSlateColor(StateColor)
            );
        }
    }

    UpdateActivityChart(Data.ActivityHistory);
}

void UStudioPulseDashboardWidget::
UpdateActivityChart(
    const TArray<float>& Values
)
{
    const int32 Count =
        FMath::Min(
            Values.Num(),
            ActivityBarSlots.Num()
        );

    for (int32 Index = 0; Index < Count; ++Index)
    {
        UCanvasPanelSlot* ActivityCanvasSlot =
            ActivityBarSlots[Index];

        if (!ActivityCanvasSlot)
        {
            continue;
        }

        const float Height =
            FMath::Clamp(
                Values[Index],
                4.0f,
                100.0f
            ) * 3.25f;

        const FVector2D CurrentPosition =
            ActivityCanvasSlot->GetPosition();

        ActivityCanvasSlot->SetPosition(
            FVector2D(
                CurrentPosition.X,
                360.0f - Height
            )
        );

        ActivityCanvasSlot->SetSize(
            FVector2D(26.0f, Height)
        );
    }
}