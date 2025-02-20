#include <gtest/gtest.h>

#include <logcollector.hpp>
#include <logcollector_mock.hpp>
#include <macos_reader.hpp>

#include <boost/asio.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace macos_reader_tests
{
    constexpr time_t DEFAULT_WAIT_IN_MILLIS = 50;

    using namespace logcollector;
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::Return;

    class MockIOSLogStoreWrapper : public IOSLogStoreWrapper
    {
    public:
        MOCK_METHOD(std::vector<LogEntry>,
                    AllEntries,
                    (const double startTimeSeconds, const std::string& query, const LogLevel logLevel),
                    (override));
    };

    TEST(MacOSReaderTest, Constructor)
    {
        auto logCollector = LogcollectorMock();
        logcollector::MacOSReader macOSReader(logCollector, DEFAULT_WAIT_IN_MILLIS);
    }

    TEST(MacOSReaderTest, ConstructorWithStoreWrapper)
    {
        auto logCollector = LogcollectorMock();
        auto logStoreMock = std::make_unique<MockIOSLogStoreWrapper>();
        MacOSReader reader(std::move(logStoreMock), logCollector, DEFAULT_WAIT_IN_MILLIS);

        // Just ensure the constructor works
        SUCCEED();
    }

    TEST(MacOSReaderTest, RunAndStop)
    {
        auto logCollector = LogcollectorMock();
        EXPECT_CALL(logCollector, Wait(::testing::_)).Times(::testing::AnyNumber());

        boost::asio::io_context ioContext;

        auto didMacOSReaderRun = false;

        logcollector::MacOSReader macOSReader(logCollector, DEFAULT_WAIT_IN_MILLIS);
        boost::asio::co_spawn(
            ioContext,
            [&macOSReader, &didMacOSReaderRun]()
                -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            {
                co_await macOSReader.Run();
                didMacOSReaderRun = true;
            },
            boost::asio::detached);

        boost::asio::post(ioContext,
                          [&macOSReader]() -> void
                          {
                              const auto sleepTime = std::chrono::seconds(1);
                              std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                              macOSReader.Stop();
                          });

        std::thread ioThread([&ioContext]() -> void { ioContext.run(); });

        ioContext.run();

        EXPECT_TRUE(didMacOSReaderRun);

        ioThread.join();
    }

    TEST(MacOSReaderTest, CreateQueryFromRule)
    {
        auto logCollector = LogcollectorMock();
        EXPECT_CALL(logCollector, Wait(::testing::_)).Times(::testing::AnyNumber());

        logcollector::MacOSReader macOSReader(logCollector,
                                              DEFAULT_WAIT_IN_MILLIS,
                                              "debug",
                                              R"(process != "sshd" OR message CONTAINS "invalid")",
                                              {"trace", "activity", "log"});

        logcollector::MacOSReader macOSReader2(logCollector,
                                               DEFAULT_WAIT_IN_MILLIS,
                                               "debug",
                                               R"(process != "wazuh-agent"")",
                                               {"trace", "activity", "log"});

        boost::asio::io_context ioContext;

        boost::asio::co_spawn(
            ioContext,
            [&macOSReader]()
                -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            { co_await macOSReader.Run(); },
            boost::asio::detached);

        boost::asio::co_spawn(
            ioContext,
            [&macOSReader2]()
                -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            { co_await macOSReader2.Run(); },
            boost::asio::detached);

        boost::asio::post(ioContext,
                          [&macOSReader, &macOSReader2]() -> void
                          {
                              std::this_thread::sleep_for(std::chrono::seconds(1));
                              macOSReader.Stop();
                              macOSReader2.Stop();
                          });

        std::thread ioThread([&ioContext]() -> void { ioContext.run(); });

        ioContext.run();
        ioThread.join();
    }

    TEST(MacOSReaderTest, ReadsEntriesCorrectly)
    {
        auto logCollector = LogcollectorMock();
        EXPECT_CALL(logCollector, Wait(::testing::_)).Times(::testing::AnyNumber());

        const double aPointInTime = 1672531200.0;
        const double anotherPointInTime = 1672531260.0;

        auto logStoreMock = std::make_unique<MockIOSLogStoreWrapper>();
        EXPECT_CALL(*logStoreMock, AllEntries(_, _, _))
            .WillOnce(Return(std::vector<IOSLogStoreWrapper::LogEntry> {
                {aPointInTime, "2023-01-01T00:00:00Z", "Sample log message 1"},
                {anotherPointInTime, "2023-01-01T00:01:00Z", "Sample log message 2"}}))
            .WillRepeatedly(Return(std::vector<IOSLogStoreWrapper::LogEntry> {}));

        MacOSReader macOSReader(std::move(logStoreMock), logCollector, DEFAULT_WAIT_IN_MILLIS);

        int pushMessageCallCount = 0;

        logCollector.SetPushMessageFunction(
            [&pushMessageCallCount, &macOSReader](Message message) -> int // NOLINT(performance-unnecessary-value-param)
            {
                EXPECT_EQ(message.moduleName, "logcollector");
                const auto dumpedData = message.data.dump();

                EXPECT_THAT(dumpedData, ::testing::HasSubstr("2023-01-01T00"));
                EXPECT_THAT(dumpedData, ::testing::HasSubstr("Sample log message "));

                ++pushMessageCallCount;
                macOSReader.Stop();
                return 0;
            });

        boost::asio::io_context ioContext;

        boost::asio::co_spawn(
            ioContext,
            [&macOSReader]()
                -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            { co_await macOSReader.Run(); },
            boost::asio::detached);

        ioContext.run();
        EXPECT_EQ(pushMessageCallCount, 2);
    }

} // namespace macos_reader_tests
