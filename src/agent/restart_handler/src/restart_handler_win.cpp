#include <restart_handler.hpp>

#include "../../src/windows/windows_service.hpp"
#include <logger.hpp>

#include <algorithm>
#include <fmt/format.h>

namespace restart_handler
{

    std::vector<char*> RestartHandler::startupCmdLineArgs;

    void ForegroundRestart()
    {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::vector<char*> cmdLineArgs;
        std::copy_if(RestartHandler::startupCmdLineArgs.begin(),
                     RestartHandler::startupCmdLineArgs.end(),
                     std::back_inserter(cmdLineArgs),
                     [](char* ptr) { return ptr != nullptr; });

        const std::string cmd = fmt::format("cmd.exe /c timeout /t 3 && {}", fmt::join(cmdLineArgs, " "));

        if (CreateProcess(NULL,
                          (LPSTR)cmd.c_str(),
                          NULL,
                          NULL,
                          FALSE,
                          CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                          NULL,
                          NULL,
                          &si,
                          &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartHandler::RestartAgent()
    {
        LogInfo("Restarting Wazuh Agent");

        bool serviceRestart =
            std::any_of(RestartHandler::startupCmdLineArgs.begin(),
                        RestartHandler::startupCmdLineArgs.end(),
                        [](const char* param) { return param != nullptr && std::strcmp(param, "--run-service") == 0; });

        if (serviceRestart)
        {
            windows_service::ServiceRestart();
        }
        else
        {
            ForegroundRestart();
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        }
        LogInfo("Exiting Wazuh Agent now");
        co_return module_command::CommandExecutionResult {module_command::Status::IN_PROGRESS,
                                                          "Pending restart execution"};
    }

} // namespace restart_handler
