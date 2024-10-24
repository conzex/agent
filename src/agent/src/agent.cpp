#include <agent.hpp>

#ifdef ENABLE_INVENTORY
#include <inventory.hpp>
#endif

#ifdef ENABLE_LOGCOLLECTOR
#include <logcollector.hpp>
using logcollector::Logcollector;
#endif

#include <command_handler_utils.hpp>
#include <http_client.hpp>
#include <message.hpp>
#include <message_queue_utils.hpp>
#include <module_command/command_entry.hpp>
#include <signal_handler.hpp>

#include <filesystem>
#include <memory>
#include <thread>

using logcollector::Logcollector;

Agent::Agent(const std::string& configPath, std::unique_ptr<ISignalHandler> signalHandler)
    : m_messageQueue(std::make_shared<MultiTypeQueue>())
    , m_signalHandler(std::move(signalHandler))
    , m_configurationParser(configPath.empty() ? configuration::ConfigurationParser()
                                               : configuration::ConfigurationParser(std::filesystem::path(configPath)))
    , m_communicator(std::make_unique<http_client::HttpClient>(),
                     m_agentInfo.GetUUID(),
                     m_agentInfo.GetKey(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser.GetConfig<std::string>(std::move(table), std::move(key)); })
    , m_moduleManager(m_messageQueue, m_configurationParser)
{
    m_centralizedConfiguration.SetGroupIdFunction([this](const std::vector<std::string>& groups)
                                                  { return m_agentInfo.SetGroups(groups); });

    m_centralizedConfiguration.GetGroupIdFunction([this]() { return m_agentInfo.GetGroups(); });

    m_centralizedConfiguration.SetDownloadGroupFilesFunction(
        [this](const std::string& groupId, const std::string& destinationPath)
        { return m_communicator.GetGroupConfigurationFromManager(groupId, destinationPath); });

    m_taskManager.Start(std::thread::hardware_concurrency());
}

Agent::~Agent()
{
    m_taskManager.Stop();
}

void Agent::Run()
{
    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(
        [this](const std::string& response) { PushCommandsToQueue(m_messageQueue, response); }));

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
        [this]() { return GetMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); },
        [this]([[maybe_unused]] const std::string& response)
        { PopMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); }));

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
        [this]() { return GetMessagesFromQueue(m_messageQueue, MessageType::STATELESS); },
        [this]([[maybe_unused]] const std::string& response)
        { PopMessagesFromQueue(m_messageQueue, MessageType::STATELESS); }));

    m_taskManager.EnqueueTask(m_commandHandler.CommandsProcessingTask<module_command::CommandEntry>(
        [this]() { return GetCommandFromQueue(m_messageQueue); },
        [this]() { return PopCommandFromQueue(m_messageQueue); },
        [this](module_command::CommandEntry& cmd)
        { return DispatchCommand(cmd, m_moduleManager.GetModule(cmd.Module), m_messageQueue); }));

#ifdef ENABLE_INVENTORY
    m_moduleManager.AddModule(Inventory::Instance());
#endif

#ifdef ENABLE_LOGCOLLECTOR
    m_moduleManager.AddModule(Logcollector::Instance());
#endif

    m_moduleManager.AddModule(m_centralizedConfiguration);
    m_moduleManager.Setup();
    m_taskManager.EnqueueTask([this]() { m_moduleManager.Start(); });

    m_signalHandler->WaitForSignal();
    m_moduleManager.Stop();
    m_communicator.Stop();
}
