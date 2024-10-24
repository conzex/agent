# Project name
set(PROJECT_NAME "Wazuh Agent")

# Project version
set(VERSION "0.1")

# Reading buffer size
set(BUFFER_SIZE 4096)

# Default Logcollector file reading interval (ms)
set(DEFAULT_FILE_WAIT 500)

# Default Logcollector reload interval (sec)
set(DEFAULT_RELOAD_INTERVAL 60)

option(ENABLE_LOGCOLLECTOR "Enable Logcollector module" ON)
option(ENABLE_INVENTORY "Enable Inventory module" OFF)