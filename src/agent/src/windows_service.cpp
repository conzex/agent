#include <windows_service.hpp>

#include <agent.hpp>
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
    const std::string AGENT_SERVICENAME = "Wazuh Agent";
    const std::string AGENT_SERVICEDESCRIPTION = "Wazuh Windows Agent";

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
            g_ServiceStatus.dwControlsAccepted = 0;
        else
            g_ServiceStatus.dwControlsAccepted =
                SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PARAMCHANGE;

        if ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED))
            g_ServiceStatus.dwCheckPoint = 0;
        else
            g_ServiceStatus.dwCheckPoint = dwCheckPoint++;

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

    std::string ServiceStatusToString(DWORD currentState)
    {
        static const std::unordered_map<DWORD, std::string> statusMap = {
            {SERVICE_STOPPED, "Service is stopped."},
            {SERVICE_START_PENDING, "Service is starting..."},
            {SERVICE_STOP_PENDING, "Service is stopping..."},
            {SERVICE_RUNNING, "Service is running."},
            {SERVICE_CONTINUE_PENDING, "Service is resuming..."},
            {SERVICE_PAUSE_PENDING, "Service is pausing..."},
            {SERVICE_PAUSED, "Service is paused."}};

        auto it = statusMap.find(currentState);
        if (it != statusMap.end())
        {
            return it->second;
        }
        else
        {
            return "Unknown service status.";
        }
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
    bool InstallService()
    {
        const std::string exePath = GetExecutablePath() + " service";

        SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService = CreateService(schSCManager,
                                             AGENT_SERVICENAME.c_str(),
                                             AGENT_SERVICENAME.c_str(),
                                             SERVICE_ALL_ACCESS,
                                             SERVICE_WIN32_OWN_PROCESS,
                                             SERVICE_AUTO_START,
                                             SERVICE_ERROR_NORMAL,
                                             exePath.c_str(),
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr);

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

        LogInfo("Wazuh Agent Service successfully installed.");

        return true;
    }

    bool RemoveService()
    {
        SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService = OpenService(schSCManager, AGENT_SERVICENAME.c_str(), DELETE);
        if (!schService)
        {
            LogError("OpenService fail: {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return false;
        }

        if (!DeleteService(schService))
        {
            LogError("DeleteService fail: {}", GetLastError());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        LogInfo("Wazuh Agent Service successfully removed.");

        return true;
    }

    void SetDispatcherThread()
    {
        SERVICE_TABLE_ENTRY ServiceTable[] = {{(LPSTR)AGENT_SERVICENAME.c_str(), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
                                              {nullptr, nullptr}};

        if (!StartServiceCtrlDispatcher(ServiceTable))
        {
            LogError("Error: StartServiceCtrlDispatcher {}", GetLastError());
        }
    }

    void WINAPI ServiceMain()
    {
        g_StatusHandle = RegisterServiceCtrlHandler(AGENT_SERVICENAME.c_str(), ServiceCtrlHandler);

        if (!g_StatusHandle)
        {
            LogError("Failed to register ServiceCtrlHandler. Error: {}", GetLastError());
            return;
        }

        g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        g_ServiceStatus.dwServiceSpecificExitCode = 0;

        ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

        g_ServiceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!g_ServiceStopEvent)
        {
            LogError("Failed to create stop event. Error: {}", GetLastError());
            ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
            return;
        }

        ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

        LogInfo("Starting Wazuh Agent.");

        Agent agent;
        agent.Run();

        WaitForSingleObject(g_ServiceStopEvent, INFINITE);

        CloseHandle(g_ServiceStopEvent);
        ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
    }

    void WINAPI ServiceCtrlHandler(DWORD ctrlCode)
    {
        switch (ctrlCode)
        {
            case SERVICE_CONTROL_STOP:
                HandleStopSignal("Wazuh Agent is stopping. Performing cleanup.", ctrlCode);
                break;
            case SERVICE_CONTROL_SHUTDOWN:
                HandleStopSignal("System is shutting down. Performing cleanup.", ctrlCode);
                break;
            case SERVICE_CONTROL_PARAMCHANGE:
                // TO DO
                break;
            default: break;
        }
    }

    void ServiceStart()
    {
        ServiceHandle hService;
        ServiceHandle hSCManager;

        if (!GetService(hSCManager, hService, SERVICE_START))
            return;

        if (!::StartService(hService.get(), 0, nullptr))
        {
            LogError("Error: Unable to start service. Error: {}", GetLastError());
        }
        else
        {
            LogInfo("Service {} started successfully.", AGENT_SERVICENAME.c_str());
        }
    }

    void ServiceStop()
    {
        ServiceHandle hService;
        ServiceHandle hSCManager;

        if (!GetService(hSCManager, hService, SERVICE_STOP))
            return;

        SERVICE_STATUS serviceStatus = {};
        if (!ControlService(hService.get(), SERVICE_CONTROL_STOP, &serviceStatus))
        {
            LogError("Error: Unable to stop service. Error: {}", GetLastError());
        }
        else
        {
            LogInfo("Service {} stopped successfully.", AGENT_SERVICENAME.c_str());
        }
    }

    void ServiceRestart()
    {
        ServiceStop();
        ServiceStart();
    }

    void ServiceStatus()
    {
        ServiceHandle hService;
        ServiceHandle hSCManager;

        if (!GetService(hSCManager, hService, SERVICE_QUERY_STATUS))
            return;

        SERVICE_STATUS_PROCESS serviceStatus;
        DWORD bytesNeeded;
        if (!QueryServiceStatusEx(hService.get(),
                                  SC_STATUS_PROCESS_INFO,
                                  (LPBYTE)&serviceStatus,
                                  sizeof(SERVICE_STATUS_PROCESS),
                                  &bytesNeeded))
        {
            LogError("Error: Unable to query service status. Error: {}", GetLastError());
        }
        else
        {
            LogInfo("{}", ServiceStatusToString(serviceStatus.dwCurrentState));
        }
    }
} // namespace WindowsService
