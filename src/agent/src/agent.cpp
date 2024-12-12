#include <agent.hpp>

#include <command_handler_utils.hpp>
#include <config.h>
#include <http_client.hpp>
#include <message.hpp>
#include <message_queue_utils.hpp>
#include <module_command/command_entry.hpp>

#include <filesystem>
#include <memory>

Agent::Agent(const std::string& configFilePath, std::unique_ptr<ISignalHandler> signalHandler)
    : m_configurationParser(configFilePath.empty() ? std::make_shared<configuration::ConfigurationParser>()
                                                   : std::make_shared<configuration::ConfigurationParser>(
                                                         std::filesystem::path(configFilePath)))
    , m_dataPath(
          m_configurationParser->GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH))
    , m_messageQueue(std::make_shared<MultiTypeQueue>(m_dataPath))
    , m_signalHandler(std::move(signalHandler))
    , m_agentInfo(
          m_dataPath, [this]() { return m_sysInfo.os(); }, [this]() { return m_sysInfo.networks(); })
    , m_communicator(
          std::make_unique<http_client::HttpClient>(),
          m_agentInfo.GetUUID(),
          m_agentInfo.GetKey(),
          [this]() { return m_agentInfo.GetHeaderInfo(); },
          [this]<typename T>(std::string table, std::string key) -> std::optional<T>
          { return m_configurationParser->GetConfig<T>(std::move(table), std::move(key)); })
    , m_moduleManager([this](Message message) -> int { return m_messageQueue->push(std::move(message)); },
                      m_configurationParser,
                      m_agentInfo.GetUUID())
    , m_commandHandler(m_dataPath)
{
    // Check if agent is registered
    if (m_agentInfo.GetName().empty() || m_agentInfo.GetKey().empty() || m_agentInfo.GetUUID().empty())
    {
        LogCritical("The agent is not registered");
        throw std::runtime_error("The agent is not registered");
    }

    m_configurationParser->SetGetGroupIdsFunction([this]() { return m_agentInfo.GetGroups(); });

    m_centralizedConfiguration.SetGroupIdFunction(
        [this](const std::vector<std::string>& groups)
        {
            m_agentInfo.SetGroups(groups);
            return m_agentInfo.SaveGroups();
        });

    m_centralizedConfiguration.GetGroupIdFunction([this]() { return m_agentInfo.GetGroups(); });

    m_centralizedConfiguration.SetDownloadGroupFilesFunction(
        [this](const std::string& groupId, const std::string& destinationPath)
        { return m_communicator.GetGroupConfigurationFromManager(groupId, destinationPath); });

    m_centralizedConfiguration.ValidateFileFunction([this](const std::filesystem::path& fileToValidate)
                                                    { return m_configurationParser->isValidYamlFile(fileToValidate); });

    m_centralizedConfiguration.ReloadModulesFunction([this]() { ReloadModules(); });

    auto agentThreadCount =
        m_configurationParser->GetConfig<size_t>("agent", "thread_count").value_or(config::DEFAULT_THREAD_COUNT);

    if (agentThreadCount < config::DEFAULT_THREAD_COUNT)
    {
        LogWarn("thread_count must be greater than {}. Using default value.", config::DEFAULT_THREAD_COUNT);
        agentThreadCount = config::DEFAULT_THREAD_COUNT;
    }

    m_taskManager.Start(agentThreadCount);
}

Agent::~Agent()
{
    m_taskManager.Stop();
}

void Agent::ReloadModules()
{
    LogInfo("Reloading Modules");
    m_configurationParser->ReloadConfiguration();
    m_moduleManager.Stop();
    m_moduleManager.Setup();
    m_moduleManager.Start();
}

void Agent::Run()
{
    // Check if the server recognizes the agent
    m_communicator.SendAuthenticationRequest();

    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate(), "Authenticate");

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager([this](const int, const std::string& response)
                                                                    { PushCommandsToQueue(m_messageQueue, response); }),
                              "FetchCommands");

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
                                  [this](const int numMessages)
                                  {
                                      return GetMessagesFromQueue(m_messageQueue,
                                                                  MessageType::STATEFUL,
                                                                  numMessages,
                                                                  [this]()
                                                                  { return m_agentInfo.GetMetadataInfo(false); });
                                  },
                                  [this]([[maybe_unused]] const int messageCount, const std::string&)
                                  { PopMessagesFromQueue(m_messageQueue, MessageType::STATEFUL, messageCount); }),
                              "Stateful");

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
                                  [this](const int numMessages)
                                  {
                                      return GetMessagesFromQueue(m_messageQueue,
                                                                  MessageType::STATELESS,
                                                                  numMessages,
                                                                  [this]()
                                                                  { return m_agentInfo.GetMetadataInfo(false); });
                                  },
                                  [this]([[maybe_unused]] const int messageCount, const std::string&)
                                  { PopMessagesFromQueue(m_messageQueue, MessageType::STATELESS, messageCount); }),
                              "Stateless");

    m_moduleManager.AddModules();
    m_moduleManager.Start();

    m_taskManager.EnqueueTask(
        m_commandHandler.CommandsProcessingTask<module_command::CommandEntry>(
            [this]() { return GetCommandFromQueue(m_messageQueue); },
            [this]() { return PopCommandFromQueue(m_messageQueue); },
            [this](const module_command::CommandEntry& cmd) { return ReportCommandResult(cmd, m_messageQueue); },
            [this](module_command::CommandEntry& cmd)
            {
                if (cmd.Module == "CentralizedConfiguration")
                {
                    return DispatchCommand(
                        cmd,
                        [this](std::string command, nlohmann::json parameters) {
                            return m_centralizedConfiguration.ExecuteCommand(std::move(command), std::move(parameters));
                        },
                        m_messageQueue);
                }
                return DispatchCommand(cmd, m_moduleManager.GetModule(cmd.Module), m_messageQueue);
            }),
        "CommandsProcessing");

    m_signalHandler->WaitForSignal();

    m_commandHandler.Stop();
    m_communicator.Stop();
    m_moduleManager.Stop();
}
