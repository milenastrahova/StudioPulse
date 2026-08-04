#include "VirtualStudioCoreEditorModule.h"

#include "Framework/Commands/UIAction.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "VirtualStudioCoreSettings.h"
#include "VirtualStudioTelemetryParser.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "VirtualStudioCoreEditor"

void FVirtualStudioCoreEditorModule::StartupModule()
{
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::
            CreateRaw(
                this,
                &FVirtualStudioCoreEditorModule::
                    RegisterMenus
            )
    );
}

void FVirtualStudioCoreEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
}

void FVirtualStudioCoreEditorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* ToolsMenu =
        UToolMenus::Get()->ExtendMenu(
            TEXT("LevelEditor.MainMenu.Tools")
        );

    FToolMenuSection& Section =
        ToolsMenu->FindOrAddSection(
            TEXT("VirtualStudioCore"),
            LOCTEXT(
                "VirtualStudioCoreSection",
                "Virtual Studio Core"
            )
        );

    Section.AddMenuEntry(
        TEXT("VirtualStudioCoreOpenSettings"),
        LOCTEXT(
            "OpenSettingsLabel",
            "Virtual Studio Core Settings"
        ),
        LOCTEXT(
            "OpenSettingsTooltip",
            "Open the HTTP, WebSocket, fallback "
            "and reconnect settings."
        ),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateRaw(
                this,
                &FVirtualStudioCoreEditorModule::
                    OpenPluginSettings
            )
        )
    );

    Section.AddMenuEntry(
        TEXT("VirtualStudioCoreSelfTest"),
        LOCTEXT(
            "SelfTestLabel",
            "Validate Virtual Studio Configuration"
        ),
        LOCTEXT(
            "SelfTestTooltip",
            "Validate configured endpoint schemes "
            "without opening a network connection."
        ),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateRaw(
                this,
                &FVirtualStudioCoreEditorModule::
                    RunConfigurationSelfTest
            )
        )
    );
}

void FVirtualStudioCoreEditorModule::
    OpenPluginSettings()
{
    ISettingsModule& SettingsModule =
        FModuleManager::LoadModuleChecked<
            ISettingsModule
        >(TEXT("Settings"));

    SettingsModule.ShowViewer(
        TEXT("Project"),
        TEXT("Plugins"),
        TEXT("Virtual Studio Core")
    );
}

void FVirtualStudioCoreEditorModule::
    RunConfigurationSelfTest()
{
    const UVirtualStudioCoreSettings* Settings =
        GetDefault<UVirtualStudioCoreSettings>();

    const bool bHttpValid =
        Settings &&
        (
            !Settings->bEnableHttpSnapshot ||
            FVirtualStudioTelemetryParser::
                IsSupportedHttpEndpoint(
                    Settings->SnapshotUrl
                )
        );

    const bool bWebSocketValid =
        Settings &&
        (
            !Settings->bEnableWebSocket ||
            FVirtualStudioTelemetryParser::
                IsSupportedWebSocketEndpoint(
                    Settings->WebSocketUrl
                )
        );

    const bool bValid =
        Settings &&
        bHttpValid &&
        bWebSocketValid;

    const FText Message =
        bValid
            ? LOCTEXT(
                "ConfigurationValid",
                "Virtual Studio Core configuration "
                "is valid."
            )
            : LOCTEXT(
                "ConfigurationInvalid",
                "Virtual Studio Core configuration "
                "contains an invalid endpoint."
            );

    FNotificationInfo Notification(Message);
    Notification.ExpireDuration = 6.0f;
    Notification.bUseSuccessFailIcons = true;

    TSharedPtr<SNotificationItem> Item =
        FSlateNotificationManager::Get().
            AddNotification(Notification);

    if (Item.IsValid())
    {
        Item->SetCompletionState(
            bValid
                ? SNotificationItem::
                    CS_Success
                : SNotificationItem::
                    CS_Fail
        );
    }
}

IMPLEMENT_MODULE(
    FVirtualStudioCoreEditorModule,
    VirtualStudioCoreEditor
)

#undef LOCTEXT_NAMESPACE
