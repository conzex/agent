#include <moduleManager.hpp>

#ifdef ENABLE_INVENTORY
#include <inventory.hpp>
#endif

#ifdef ENABLE_LOGCOLLECTOR
#include <logcollector.hpp>
using logcollector::Logcollector;
#endif

namespace
{
    constexpr int MODULES_START_WAIT_SECS = 60;
}

ModuleManager::ModuleManager(const std::function<int(Message)>& pushMessage,
                             std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                             std::string uuid)
    : m_pushMessage(pushMessage)
    , m_configurationParser(std::move(configurationParser))
    , m_agentUUID(std::move(uuid))
{
    if (!m_pushMessage)
    {
        throw std::runtime_error("Invalid Push Message Function passed.");
    }

    if (!m_configurationParser)
    {
        throw std::runtime_error("Invalid Configuration Parser passed.");
    }
}

void ModuleManager::AddModules()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

#ifdef ENABLE_INVENTORY
        Inventory& inventory = Inventory::Instance();
        inventory.SetAgentUUID(m_agentUUID);
        AddModule(inventory);
#endif

#ifdef ENABLE_LOGCOLLECTOR
        AddModule(Logcollector::Instance());
#endif
    }

    Setup();
}

std::shared_ptr<ModuleWrapper> ModuleManager::GetModule(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_modules.find(name); it != m_modules.end())
    {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::Start()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_taskManager.Start(m_modules.size());

    m_started.store(0);

    for (const auto& [_, module] : m_modules)
    {
        m_taskManager.EnqueueTask(
            [this, module]
            {
                ++m_started;
                module->Start();
            },
            module->Name());
    }

    const auto start = std::chrono::steady_clock::now();

    while (m_started.load() != static_cast<int>(m_modules.size()))
    {
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        if (elapsed_seconds.count() > MODULES_START_WAIT_SECS)
        {
            break;
        }

        const auto sleepTime = std::chrono::milliseconds(10);
        std::this_thread::sleep_for(sleepTime);
    }

    if (m_started.load() != static_cast<int>(m_modules.size()))
    {
        LogError("Error when starting some modules. Modules started: {}", m_started.load());
    }
}

void ModuleManager::Setup()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [_, module] : m_modules)
    {
        module->Setup(m_configurationParser);
    }
}

void ModuleManager::Stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [_, module] : m_modules)
    {
        module->Stop();
    }
    m_taskManager.Stop();
}
