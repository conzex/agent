#include <agent.hpp>

#include <chrono>
#include <thread>

Agent::Agent()
    : m_configurationParser(std::make_unique<configuration::ConfigurationParser>())
    , m_communicator(m_agentInfo.GetUUID(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser->GetConfig<std::string>(table, key); })
{
    m_taskManager.start(std::thread::hardware_concurrency());

    m_taskManager.enqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());
    m_taskManager.enqueueTask(m_communicator.GetCommandsFromManager());
    m_taskManager.enqueueTask(m_communicator.StatefulMessageProcessingTask(m_messageQueue));
    m_taskManager.enqueueTask(m_communicator.StatelessMessageProcessingTask(m_messageQueue));
}

Agent::~Agent()
{
    m_communicator.Stop();
    m_taskManager.stop();
}
