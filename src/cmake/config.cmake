# Project name
set(PROJECT_NAME "Wazuh Agent")

# Project version
set(VERSION "0.1")

set(DEFAULT_THREAD_COUNT 4 CACHE STRING "Default number of threads (4)")

string(REPLACE "\\" "\\\\" DEFAULT_DATA_PATH "${DATA_INSTALL_DIR}")

string(REPLACE "\\" "\\\\" DEFAULT_RUN_PATH "${RUN_INSTALL_DIR}")

string(REPLACE "\\" "\\\\" DEFAULT_SHARED_CONFIG_PATH "${SHARED_CONFIG_INSTALL_DIR}")

string(REPLACE "\\" "\\\\" DEFAULT_CONFIG_PATH "${CONFIG_INSTALL_DIR}")

set(DEFAULT_SHARED_FILE_EXTENSION ".yml" CACHE STRING "Default shared file extension")

set(DEFAULT_SERVER_URL "https://localhost:27000" CACHE STRING "Default Agent Server Url")

set(DEFAULT_RETRY_INTERVAL 30000 CACHE STRING "Default Agent retry interval (30s)")

set(DEFAULT_BATCH_INTERVAL 10000 CACHE STRING "Default Agent batch interval (10s)")

set(DEFAULT_BATCH_SIZE 1000000ULL CACHE STRING "Default Agent batch size limit (1MB)")

set(DEFAULT_VERIFICATION_MODE "full" CACHE STRING "Default Agent verification mode")

set(DEFAULT_LOGCOLLECTOR_ENABLED true CACHE BOOL "Default Logcollector enabled")

set(BUFFER_SIZE 4096 CACHE STRING "Default Logcollector reading buffer size")

set(DEFAULT_FILE_WAIT 500 CACHE STRING "Default Logcollector file reading interval (500ms)")

set(DEFAULT_RELOAD_INTERVAL 60000 CACHE STRING "Default Logcollector reload interval (1m)")

set(DEFAULT_RECONNECT_TIME 5000 CACHE STRING "Default Logcollector reconnect time (5000ms)")

set(DEFAULT_USE_BOOKMARK false CACHE BOOL "Default Logcollector windows bookmark enabled (false)")

set(DEFAULT_INVENTORY_ENABLED true CACHE BOOL "Default inventory enabled")

set(DEFAULT_INTERVAL 3600000 CACHE STRING "Default inventory interval (1h)")

set(DEFAULT_SCAN_ON_START true CACHE BOOL "Default inventory scan on start")

set(DEFAULT_HARDWARE true CACHE BOOL "Default inventory hardware")

set(DEFAULT_OS true CACHE BOOL "Default inventory os")

set(DEFAULT_NETWORK true CACHE BOOL "Default inventory network")

set(DEFAULT_PACKAGES true CACHE BOOL "Default inventory packages")

set(DEFAULT_PORTS true CACHE BOOL "Default inventory ports")

set(DEFAULT_PORTS_ALL true CACHE BOOL "Default inventory ports all")

set(DEFAULT_PROCESSES true CACHE BOOL "Default inventory processes")

set(DEFAULT_HOTFIXES true CACHE BOOL "Default inventory hotfixes")

set(QUEUE_STATUS_REFRESH_TIMER 100 CACHE STRING "Default Agent's queue refresh timer (100ms)")

set(QUEUE_DEFAULT_SIZE 10000 CACHE STRING "Default Agent's queue size (10000)")
