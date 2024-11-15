#include <cstdio>
#include <gtest/gtest.h>
#include "inventoryImp_test.hpp"
#include "inventory.hpp"

constexpr auto INVENTORY_DB_PATH {"TEMP.db"};
constexpr int SLEEP_DURATION_SECONDS = 10;

void ReportFunction(const std::string& payload);

void InventoryImpTest::SetUp() {};

void InventoryImpTest::TearDown()
{
    std::remove(INVENTORY_DB_PATH);
};

using ::testing::Return;

class SysInfoWrapper: public ISysInfo
{
    public:
        SysInfoWrapper() = default;
        ~SysInfoWrapper() override = default;

        SysInfoWrapper(const SysInfoWrapper&) = delete;
        SysInfoWrapper& operator=(const SysInfoWrapper&) = delete;
        SysInfoWrapper(SysInfoWrapper&&) = delete;
        SysInfoWrapper& operator=(SysInfoWrapper&&) = delete;

        MOCK_METHOD(nlohmann::json, hardware, (), (override));
        MOCK_METHOD(nlohmann::json, packages, (), (override));
        MOCK_METHOD(void, packages, (std::function<void(nlohmann::json&)>), (override));
        MOCK_METHOD(nlohmann::json, os, (), (override));
        MOCK_METHOD(nlohmann::json, networks, (), (override));
        MOCK_METHOD(nlohmann::json, processes, (), (override));
        MOCK_METHOD(void, processes, (std::function<void(nlohmann::json&)>), (override));
        MOCK_METHOD(nlohmann::json, ports, (), (override));
        MOCK_METHOD(nlohmann::json, hotfixes, (), (override));
};

class CallbackMock
{
    public:
        CallbackMock() = default;
        ~CallbackMock() = default;

        CallbackMock(const CallbackMock&) = delete;
        CallbackMock& operator=(const CallbackMock&) = delete;
        CallbackMock(CallbackMock&&) = delete;
        CallbackMock& operator=(CallbackMock&&) = delete;

        MOCK_METHOD(void, callbackMock, (const std::string&), ());
};

void ReportFunction(const std::string& /*payload*/)
{
    // std::cout << payload << std::endl;
}

TEST_F(InventoryImpTest, defaultCtor)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    const configuration::ConfigurationParser configurationParser;
    Inventory::Instance().Setup(configurationParser);
    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, intervalSeconds)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(2))
    .WillRepeatedly(::testing::InvokeArgument<0>
                    (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 1
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          ReportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");

        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{SLEEP_DURATION_SECONDS});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noScanOnStart)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).Times(0);
    EXPECT_CALL(*spInfoWrapper, os()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);
    EXPECT_CALL(*spInfoWrapper, processes(testing::_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);
    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: false
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        ReportFunction,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHardware)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).Times(0);
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noOs)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: false
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(2));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noNetwork)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));


    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: true
            network: false
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPackages)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));

    EXPECT_CALL(*spInfoWrapper, packages()).Times(0);


    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: false
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPorts)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 5
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: false
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPortsAll)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: false
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noProcesses)
{

    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: false
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHotfixes)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":0},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"id":"aW52ZW50b3J5OmhhcmR3YXJlOkludGVsIENvcnBvcmF0aW9u","operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"aW52ZW50b3J5OnN5c3RlbTpNaWNyb3NvZnQgV2luZG93cyA3","operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: false
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, scanInvalidData)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"name":"TEXT", "scan_time":"2020/12/28 21:49:50", "version":"TEXT", "vendor":"TEXT", "install_time":"TEXT", "location":"TEXT", "architecture":"TEXT", "groups":"TEXT", "description":"TEXT", "size":"TEXT", "priority":"TEXT", "multiarch":"TEXT", "source":"TEXT", "os_patch":"TEXT"})"_json));


    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 60
            scan_on_start: true
            hardware: true
            os: true
            network: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          ReportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };
    std::this_thread::sleep_for(std::chrono::seconds{1});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, portAllEnable)
{
    // TO DO Enable this test when migrating the portAll table.
    GTEST_SKIP();

    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(R"(
    [
        {
            "inode":43481,
            "local_ip":"0.0.0.0",
            "local_port":47748,
            "pid":0,
            "process_name":"",
            "protocol":"udp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":43482,
            "local_ip":"::",
            "local_port":51087,
            "pid":0,
            "process_name":"",
            "protocol":"udp6",
            "remote_ip":"::",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        }
    ])")));

    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("checksum");
            delta["data"].erase("scan_time");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"inode":43481,"item_id":"12903a43db24ab10d872547cdd1d786a5876a0da","local_ip":"0.0.0.0","local_port":47748,"pid":0,"process_name":null,"protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"inode":43482,"item_id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f","local_ip":"::","local_port":51087,"pid":0,"process_name":null,"protocol":"udp6","remote_ip":"::","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"inode":50324,"item_id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29","local_ip":"127.0.0.1","local_port":33060,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    const auto expectedResult4
    {
        R"({"data":{"inode":122575,"item_id":"d5511242275bd3f2d57175f248108d6c3b39c438","local_ip":"192.168.0.104","local_port":39106,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"44.238.116.130","remote_port":443,"rx_queue":0,"state":"established","tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult4)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            os: false
            network: false
            packages: false
            ports: true
            ports_all: true
            processes: false
            hotfixes: false
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, portAllDisable)
{
    // TO DO Enable this test when migrating the portAll table.
    GTEST_SKIP();

    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(R"(
    [
        {
            "inode":43481,
            "local_ip":"0.0.0.0",
            "local_port":47748,
            "pid":0,
            "process_name":"",
            "protocol":"udp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":43482,
            "local_ip":"::",
            "local_port":51087,
            "pid":0,
            "process_name":"",
            "protocol":"udp6",
            "remote_ip":"::",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        }
    ])")));

    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("checksum");
            delta["data"].erase("scan_time");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"inode":43481,"item_id":"12903a43db24ab10d872547cdd1d786a5876a0da","local_ip":"0.0.0.0","local_port":47748,"pid":0,"process_name":null,"protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"inode":43482,"item_id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f","local_ip":"::","local_port":51087,"pid":0,"process_name":null,"protocol":"udp6","remote_ip":"::","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"inode":50324,"item_id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29","local_ip":"127.0.0.1","local_port":33060,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            os: false
            network: false
            packages: false
            ports: true
            ports_all: false
            processes: false
            hotfixes: false
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, PackagesDuplicated)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::DoAll(
                  ::testing::InvokeArgument<0>
                  (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json),
                  ::testing::InvokeArgument<0>
                  (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json)));



    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapper.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":"","name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"id":"aW52ZW50b3J5OnBhY2thZ2VzOnhzZXJ2ZXIteG9yZzoxOjcuNysxOXVidW50dTE0OmFtZDY0OmRlYjog","operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            os: false
            network: false
            packages: true
            ports: false
            ports_all: false
            processes: false
            hotfixes: false
    )";
    Inventory::Instance().Setup(configuration::ConfigurationParser(inventoryConfig));

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
