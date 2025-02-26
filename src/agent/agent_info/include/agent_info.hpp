#pragma once

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class DefendxAgentInfoPersistence;

/// @brief Stores and manages information about Defendx Agent.
///
/// This class provides methods for getting and setting the agent's name, key,
/// UUID, and groups. It also includes private methods for creating and
/// validating the key.
class DefendxAgentInfo
{
public:
    /// @brief Constructs a DefendxAgentInfo object with OS and network information retrieval functions.
    ///
    /// This constructor initializes the DefendxAgentInfo object by setting up OS and network
    /// information retrieval functions. It also generates a UUID for the agent if one
    /// does not already exist, and loads endpoint, metadata, and header information.
    ///
    /// @param dbFolderPath Path to the database folder.
    /// @param getOSInfo Function to retrieve OS information in JSON format.
    /// @param getNetworksInfo Function to retrieve network information in JSON format.
    /// @param agentIsEnrolling True if the agent is enrolling, false otherwise.
    /// @param persistence Optional pointer to a DefendxAgentInfoPersistence object.
    DefendxAgentInfo(const std::string& dbFolderPath,
                     std::function<nlohmann::json()> getOSInfo = nullptr,
                     std::function<nlohmann::json()> getNetworksInfo = nullptr,
                     bool agentIsEnrolling = false,
                     std::shared_ptr<DefendxAgentInfoPersistence> persistence = nullptr);

    /// @brief Gets the Defendx Agent's name.
    std::string GetName() const;

    /// @brief Gets the Defendx Agent's key.
    std::string GetKey() const;

    /// @brief Gets the Defendx Agent's UUID.
    std::string GetUUID() const;

    /// @brief Gets the Defendx Agent's groups.
    std::vector<std::string> GetGroups() const;

    /// @brief Sets the Defendx Agent's name.
    bool SetName(const std::string& name);

    /// @brief Sets the Defendx Agent's key.
    bool SetKey(const std::string& key);

    /// @brief Sets the Defendx Agent's UUID.
    void SetUUID(const std::string& uuid);

    /// @brief Sets the Defendx Agent's groups.
    void SetGroups(const std::vector<std::string>& groupList);

    /// @brief Gets the Defendx Agent's type.
    std::string GetType() const;

    /// @brief Gets the Defendx Agent's version.
    std::string GetVersion() const;

    /// @brief Gets the Defendx Agent information for the request header.
    std::string GetHeaderInfo() const;

    /// @brief Gets all the information about the Defendx Agent.
    std::string GetMetadataInfo() const;

    /// @brief Saves the Defendx Agent's information to the database.
    void Save() const;

    /// @brief Saves the Defendx Agent's group information to the database.
    bool SaveGroups() const;

private:
    /// @brief Creates a random key for the Defendx Agent.
    std::string CreateKey() const;

    /// @brief Validates a given key.
    bool ValidateKey(const std::string& key) const;

    /// @brief Loads the endpoint information.
    void LoadEndpointInfo();

    /// @brief Loads the header information.
    void LoadHeaderInfo();

    /// @brief Extracts the active IP addresses from the network JSON data.
    std::vector<std::string> GetActiveIPAddresses(const nlohmann::json& networksJson) const;

    std::string m_name;
    std::string m_key;
    std::string m_uuid;
    std::vector<std::string> m_groups;
    nlohmann::json m_endpointInfo;
    std::string m_headerInfo;
    std::function<nlohmann::json()> m_getOSInfo;
    std::function<nlohmann::json()> m_getNetworksInfo;
    bool m_agentIsEnrolling;
    std::shared_ptr<DefendxAgentInfoPersistence> m_persistence;
};
