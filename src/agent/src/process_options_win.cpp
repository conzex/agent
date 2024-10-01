#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <windows_service.hpp>

#include <vector>

void RestartAgent()
{
    WindowsService::ServiceRestart();
}

void StartAgent()
{
    WindowsService::ServiceStart();
}

void StatusAgent()
{
    WindowsService::ServiceStatus();
}

void StopAgent()
{
    WindowsService::ServiceStop();
}

bool InstallService()
{
    return WindowsService::InstallService();
}

bool RemoveService()
{
    return WindowsService::RemoveService();
}

void SetDispatcherThread()
{
    WindowsService::SetDispatcherThread();
}
