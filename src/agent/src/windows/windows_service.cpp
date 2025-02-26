#include <windows_service.hpp>

#include <agent.hpp>
#include <instance_handler.hpp>
#include <logger.hpp>
#include <signal_handler.hpp>

#include <memory>
#include <unordered_map>

namespace
{
    SERVICE_STATUS g_ServiceStatus = {};
    SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
    HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;
    SERVICE_DESCRIPTION g_serviceDescription;
    const std::string AGENT_SERVICENAME = "DefendX Agent";
    const std::string AGENT_SERVICEDESCRIPTION = "DefendX Windows Agent";

    struct ServiceHandleDeleter
    {
        void operator()(SC_HANDLE handle) const
        {
            if (handle)
            {
                CloseServiceHandle(handle);
            }
        }
    };

    using ServiceHandle = std::unique_ptr<std::remove_pointer_t<SC_HANDLE>, ServiceHandleDeleter>;

    std::string GetExecutablePath()
    {
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        return std::string(buffer);
    }

    void ReportServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint)
    {
        static DWORD dwCheckPoint = 1;

        g_ServiceStatus.dwCurrentState = currentState;
        g_ServiceStatus.dwWin32ExitCode = win32ExitCode;
        g_ServiceStatus.dwWaitHint = waitHint;

        if (currentState == SERVICE_START_PENDING)
        {
            g_ServiceStatus.dwControlsAccepted = 0;
        }
        else
        {
            g_ServiceStatus.dwControlsAccepted =
                SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PARAMCHANGE;
        }

        if ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED))
        {
            g_ServiceStatus.dwCheckPoint = 0;
        }
        else
        {
            g_ServiceStatus.dwCheckPoint = dwCheckPoint++;
        }

        if (!SetServiceStatus(g_StatusHandle, &g_ServiceStatus))
        {
            LogError("Failed to set service status to {}. Error: {}", g_ServiceStatus.dwCurrentState, GetLastError());
        }
    }

    void HandleStopSignal(const char* logMessage, DWORD ctrlCode)
    {
        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
        {
            return;
        }

        ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        SignalHandler::HandleSignal(ctrlCode);

        LogInfo("{}", logMessage);

        SetEvent(g_ServiceStopEvent);
        ReportServiceStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);
    }

    bool GetService(ServiceHandle& hSCManager, ServiceHandle& hService, DWORD desiredAccess)
    {
        hSCManager.reset(OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
        if (!hSCManager)
        {
            LogError("Error: Unable to open Service Control Manager. Error: {}", GetLastError());
            return false;
        }

        hService.reset(OpenService(hSCManager.get(), AGENT_SERVICENAME.c_str(), desiredAccess));
        if (!hService)
        {
            LogError("Error: Unable to open service.Error: {}", GetLastError());
            return false;
        }

        return true;
    }
} // namespace

namespace WindowsService
{
    bool InstallService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade)
    {
        const std::string exePath = GetExecutablePath() + " --run-service";

        SC_HANDLE schSCManager = static_cast<SC_HANDLE>(windowsApiFacade.OpenSCM(SC_MANAGER_CREATE_SERVICE));
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService = static_cast<SC_HANDLE>(
            windowsApiFacade.CreateSvc(schSCManager, AGENT_SERVICENAME.c_str(), exePath.c_str()));

        if (!schService)
        {
            LogError("CreateService fail: {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return false;
        }

        g_serviceDescription.lpDescription = const_cast<char*>(AGENT_SERVICEDESCRIPTION.c_str());
        ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &g_serviceDescription);

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        LogInfo("DefendX Agent Service successfully installed.");

        return true;
    }

    bool RemoveService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade)
    {
        SC_HANDLE schSCManager = static_cast<SC_HANDLE>(windowsApiFacade.OpenSCM(SC_MANAGER_CREATE_SERVICE));
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService =
            static_cast<SC_HANDLE>(windowsApiFacade.OpenSvc(schSCManager, AGENT_SERVICENAME.c_str(), DELETE));
        if (!schService)
        {
            LogError("OpenService fail: {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return false;
        }

        if (!windowsApiFacade.DeleteSvc(schService))
        {
            LogError("DeleteService fail: {}", GetLastError());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        LogInfo("DefendX Agent Service successfully removed.");

        return true;
    }
} // namespace WindowsService
