#pragma once

#include "Modules/ModuleManager.h"

class FVirtualStudioCoreEditorModule
    : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterMenus();
    void OpenPluginSettings();
    void RunConfigurationSelfTest();
};
