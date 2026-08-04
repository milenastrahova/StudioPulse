#include "VirtualStudioCoreSettings.h"

UVirtualStudioCoreSettings::UVirtualStudioCoreSettings()
{
    SnapshotUrl =
        TEXT("http://127.0.0.1:8090/snapshot");

    WebSocketUrl =
        TEXT("ws://127.0.0.1:8091");

    ReconnectPolicy.BaseDelaySeconds = 1.0f;
    ReconnectPolicy.MaximumDelaySeconds = 30.0f;
    ReconnectPolicy.Multiplier = 2.0f;
}

FName UVirtualStudioCoreSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}
