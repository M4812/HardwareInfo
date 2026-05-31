#pragma once

#include "Models.h"
#include "WmiClient.h"

bool IsRunningAsAdministrator();
SystemReport CollectSystemReport(WmiClient& wmi);
