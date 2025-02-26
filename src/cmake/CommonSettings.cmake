function(set_common_settings)
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(BIN_INSTALL_DIR "usr/share/defendx-agent/bin" PARENT_SCOPE)
        set(CONFIG_INSTALL_DIR "etc/defendx-agent" PARENT_SCOPE)
        set(SERVICE_INSTALL_DIR "usr/lib/systemd/system" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "var/lib/defendx-agent" PARENT_SCOPE)
        set(RUN_INSTALL_DIR "var/run" PARENT_SCOPE)
        set(SERVICE_FILE "${CMAKE_SOURCE_DIR}/agent/service/defendx-agent.service" PARENT_SCOPE)
        set(SHARED_CONFIG_INSTALL_DIR "etc/defendx-agent/shared" PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(BIN_INSTALL_DIR "Library/Application Support/DefendX agent.app/bin" PARENT_SCOPE)
        set(CONFIG_INSTALL_DIR "Library/Application Support/DefendX agent.app/etc" PARENT_SCOPE)
        set(SERVICE_INSTALL_DIR "Library/LaunchDaemons" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "Library/Application Support/DefendX agent.app/var" PARENT_SCOPE)
        set(RUN_INSTALL_DIR "private/var/run" PARENT_SCOPE)
        set(SERVICE_FILE "${CMAKE_SOURCE_DIR}/agent/service/com.defendx.agent.plist" PARENT_SCOPE)
        set(SHARED_CONFIG_INSTALL_DIR "Library/Application Support/DefendX agent.app/etc/shared" PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        add_definitions(-DNOMINMAX)

        if(DEFINED ENV{ProgramFiles})
            set(BIN_INSTALL_DIR "$ENV{ProgramFiles}\\defendx-agent" PARENT_SCOPE)
        else()
            set(BIN_INSTALL_DIR "C:\\Program Files\\defendx-agent" PARENT_SCOPE)
        endif()

        if(DEFINED ENV{ProgramData})
            set(CONFIG_INSTALL_DIR "$ENV{ProgramData}\\defendx-agent\\config" PARENT_SCOPE)
            set(DATA_INSTALL_DIR "$ENV{ProgramData}\\defendx-agent\\var" PARENT_SCOPE)
            set(RUN_INSTALL_DIR "$ENV{ProgramData}\\defendx-agent\\run" PARENT_SCOPE)
            set(SHARED_CONFIG_INSTALL_DIR "$ENV{ProgramData}\\defendx-agent\\config\\shared" PARENT_SCOPE)
        else()
            set(CONFIG_INSTALL_DIR "C:\\ProgramData\\defendx-agent\\config" PARENT_SCOPE)
            set(DATA_INSTALL_DIR "C:\\ProgramData\\defendx-agent\\var" PARENT_SCOPE)
            set(RUN_INSTALL_DIR "C:\\ProgramData\\defendx-agent\\run" PARENT_SCOPE)
            set(SHARED_CONFIG_INSTALL_DIR "C:\\ProgramData\\defendx-agent\\config\\shared" PARENT_SCOPE)
        endif()
    else()
        message(FATAL_ERROR "Not supported OS")
    endif()

    option(BUILD_TESTS "Enable tests building" OFF)
    option(COVERAGE "Enable coverage report" OFF)
    option(ENABLE_INVENTORY "Enable Inventory module" ON)
    option(ENABLE_LOGCOLLECTOR "Enable Logcollector module" ON)

    if(COVERAGE)
        if(NOT TARGET coverage)
            set(CMAKE_BUILD_TYPE Debug)

            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                add_compile_options(-O0 -fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
                add_link_options(-fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
            else()
                add_compile_options(-O0 --coverage)
                add_link_options(--coverage)
            endif()

            find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov)

            if(LLVM_COV_EXECUTABLE)
                set(GCOV_EXECUTABLE "llvm-cov gcov")
            else()
                set(GCOV_EXECUTABLE "gcov")
            endif()

            add_custom_target(coverage
                COMMAND ${CMAKE_MAKE_PROGRAM} test
                COMMAND gcovr --gcov-executable "${GCOV_EXECUTABLE}" -v -r .. -e '.*main\\.cpp' -e '.*vcpkg.*' -e '.*mock_.*' -e '.*_test.*' --html --html-details -o coverage.html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
        endif()
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" PARENT_SCOPE)
    endif()

    if(MSVC)
        set(CMAKE_CXX_FLAGS "/Zi /DWIN32 /EHsc" PARENT_SCOPE)
        add_compile_options(/Zc:preprocessor)
    else()
        set(CMAKE_CXX_FLAGS "-g3" PARENT_SCOPE)
    endif()

    if(ENABLE_INVENTORY)
        add_definitions(-DENABLE_INVENTORY)
    endif()

    if(ENABLE_LOGCOLLECTOR)
        add_definitions(-DENABLE_LOGCOLLECTOR)
    endif()
endfunction()

