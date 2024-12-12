#include <agent.hpp>
#include <cstdio>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <isignal_handler.hpp>

class MockSignalHandler : public ISignalHandler
{
public:
    MOCK_METHOD(void, WaitForSignal, (), (override));
};

class AgentTests : public ::testing::Test
{
protected:
    std::string AGENT_CONFIG_PATH;
    std::string AGENT_DB_PATH;
    std::string AGENT_PATH;
    std::string TMP_PATH;

    void SetUp() override
    {
#ifdef WIN32
        char* tmpPath = nullptr;
        size_t len = 0;

        _dupenv_s(&tmpPath, &len, "TMP");
        std::string tempFolder = std::string(tmpPath);
        AGENT_CONFIG_PATH = tempFolder + "wazuh-agent.yml";
        AGENT_DB_PATH = tempFolder + "agent_info.db";
        AGENT_PATH = tempFolder;
        TMP_PATH = tempFolder;
#else
        AGENT_CONFIG_PATH = "/tmp/wazuh-agent.yml";
        AGENT_DB_PATH = "/tmp/agent_info.db";
        AGENT_PATH = "/tmp/wazuh-agent";
        TMP_PATH = "/tmp";
#endif

        CreateTempConfigFile();

        SysInfo sysInfo;
        std::unique_ptr<AgentInfo> agent = std::make_unique<AgentInfo>(
            TMP_PATH,
            [&sysInfo]() mutable { return sysInfo.os(); },
            [&sysInfo]() mutable { return sysInfo.networks(); });

        agent->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
        agent->SetName("agent_name");
        agent->Save();
    }

    void TearDown() override
    {
        CleanUpTempFiles();
    }

    void CreateTempConfigFile()
    {
        CleanUpTempFiles();

        std::ofstream configFilePath(AGENT_CONFIG_PATH);
        configFilePath << R"(
agent:
  server_url: https://localhost:27000
  path.data: )" << AGENT_PATH
                       << R"(
  retry_interval: 30s
inventory:
  enabled: false
  interval: 1h
  scan_on_start: true
  hardware: true
  os: true
  network: true
  packages: true
  ports: true
  ports_all: true
  processes: true
  hotfixes: true
logcollector:
  enabled: false
  localfiles:
    - /var/log/auth.log
  reload_interval: 1m
  file_wait: 500ms
)";
        configFilePath.close();
    }

    void CleanUpTempFiles()
    {
        std::remove(AGENT_CONFIG_PATH);
        std::remove("/tmp/agent_info.db");
    }
};

TEST_F(AgentTests, AgentDefaultConstruction)
{
    EXPECT_NO_THROW(Agent {AGENT_CONFIG_PATH});
}

TEST_F(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    auto WaitForSignalCalled = false;

    EXPECT_CALL(*mockSignalHandlerPtr, WaitForSignal())
        .Times(1)
        .WillOnce([&WaitForSignalCalled]() { WaitForSignalCalled = true; });

    Agent agent(AGENT_CONFIG_PATH, std::move(mockSignalHandler));

    EXPECT_NO_THROW(agent.Run());
    EXPECT_TRUE(WaitForSignalCalled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
