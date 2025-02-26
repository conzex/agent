#include <agent_info.hpp>
#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <random>
#include <utility>

namespace
{
    constexpr size_t KEY_LENGTH = 32;
    const std::string AGENT_TYPE = "Endpoint";
    const std::string AGENT_VERSION = "5.0.0";
    const std::string PRODUCT_NAME = "DefendX XDR";
} // namespace

AgentInfo::AgentInfo(const std::string& dbFolderPath,
                     std::function<nlohmann::json()> getOSInfo,
                     std::function<nlohmann::json()> getNetworksInfo,
                     bool agentIsEnrolling,
                     std::shared_ptr<AgentInfoPersistance> persistence)
    : m_agentIsEnrolling(agentIsEnrolling)
    , m_persistence(persistence ? std::move(persistence) : std::make_shared<AgentInfoPersistance>(dbFolderPath))
{
    if (!m_agentIsEnrolling)
    {
        m_name = m_persistence->GetName();
        m_key = m_persistence->GetKey();
        m_uuid = m_persistence->GetUUID();
        m_groups = m_persistence->GetGroups();
    }

    if (m_uuid.empty())
    {
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    }

    if (getOSInfo != nullptr)
    {
        m_getOSInfo = std::move(getOSInfo);
    }

    if (getNetworksInfo != nullptr)
    {
        m_getNetworksInfo = std::move(getNetworksInfo);
    }

    LoadEndpointInfo();
    LoadHeaderInfo();
}

std::string AgentInfo::GetName() const
{
    return m_name;
}

std::string AgentInfo::GetKey() const
{
    return m_key;
}

std::string AgentInfo::GetUUID() const
{
    return m_uuid;
}

std::vector<std::string> AgentInfo::GetGroups() const
{
    return m_groups;
}

bool AgentInfo::SetName(const std::string& name)
{
    if (!name.empty())
    {
        m_name = name;
    }
    else if (m_getOSInfo != nullptr)
    {
        m_name = m_getOSInfo().value("hostname", "Unknown");
    }
    else
    {
        return false;
    }

    return true;
}

bool AgentInfo::SetKey(const std::string& key)
{
    if (!key.empty())
    {
        if (!ValidateKey(key))
        {
            return false;
        }
        m_key = key;
    }
    else
    {
        m_key = CreateKey();
    }

    return true;
}

void AgentInfo::SetUUID(const std::string& uuid)
{
    m_uuid = uuid;
}

void AgentInfo::SetGroups(const std::vector<std::string>& groupList)
{
    m_groups = groupList;
}

std::string AgentInfo::CreateKey() const
{
    constexpr std::string_view charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    std::string key;
    for (size_t i = 0; i < KEY_LENGTH; ++i)
    {
        key += charset[distribution(generator)];
    }

    return key;
}

bool AgentInfo::ValidateKey(const std::string& key) const
{
    return key.length() == KEY_LENGTH && std::ranges::all_of(key, ::isalnum);
}

std::string AgentInfo::GetType() const
{
    return AGENT_TYPE;
}

std::string AgentInfo::GetVersion() const
{
    return AGENT_VERSION;
}

std::string AgentInfo::GetHeaderInfo() const
{
    return m_headerInfo;
}

std::string AgentInfo::GetMetadataInfo() const
{
    nlohmann::json agentMetadataInfo;
    auto& target = m_agentIsEnrolling ? agentMetadataInfo : agentMetadataInfo["agent"];

    target["id"] = GetUUID();
    target["name"] = GetName();
    target["type"] = GetType();
    target["version"] = GetVersion();

    if (!m_endpointInfo.empty())
    {
        target["host"] = m_endpointInfo;
    }

    if (m_agentIsEnrolling)
    {
        target["key"] = GetKey();
    }
    else
    {
        target["groups"] = GetGroups();
    }

    return agentMetadataInfo.dump();
}

void AgentInfo::Save() const
{
    m_persistence->ResetToDefault();
    m_persistence->SetName(m_name);
    m_persistence->SetKey(m_key);
    m_persistence->SetUUID(m_uuid);
    m_persistence->SetGroups(m_groups);
}

bool AgentInfo::SaveGroups() const
{
    return m_persistence->SetGroups(m_groups);
}

void AgentInfo::LoadEndpointInfo()
{
    if (m_getOSInfo != nullptr)
    {
        const nlohmann::json osInfo = m_getOSInfo();
        m_endpointInfo["hostname"] = osInfo.value("hostname", "Unknown");
    }
}

void AgentInfo::LoadHeaderInfo()
{
    m_headerInfo = PRODUCT_NAME + "/" + GetVersion() + " (" + GetType() + ")";
}
