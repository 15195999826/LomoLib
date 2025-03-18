#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
DEFINE_LOG_CATEGORY_STATIC(LogLomoLibTests, Log, All);
class FLomoLibTestModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
