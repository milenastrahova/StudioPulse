#include "VirtualStudioCoreTypes.h"

bool FVirtualStudioTelemetryFrame::IsValid() const
{
    return OperatorCount >= 0 &&
        LatencyMs >= 0.0f &&
        UptimeSeconds >= 0.0f &&
        AlertCount >= 0 &&
        !Status.IsEmpty();
}

float FVirtualStudioReconnectPolicy::GetDelaySeconds(
    const int32 AttemptIndex
) const
{
    const int32 SafeAttempt =
        FMath::Max(0, AttemptIndex);

    const float SafeBase =
        FMath::Max(0.1f, BaseDelaySeconds);

    const float SafeMultiplier =
        FMath::Max(1.0f, Multiplier);

    const float SafeMaximum =
        FMath::Max(SafeBase, MaximumDelaySeconds);

    const float Delay =
        SafeBase *
        FMath::Pow(
            SafeMultiplier,
            static_cast<float>(SafeAttempt)
        );

    return FMath::Clamp(
        Delay,
        SafeBase,
        SafeMaximum
    );
}
