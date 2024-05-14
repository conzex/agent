'''
copyright: Copyright (C) 2015-2023, Wazuh Inc.

           Created by Wazuh, Inc. <info@wazuh.com>.

           This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

type: integration

brief: File Integrity Monitoring (FIM) system watches selected files and triggering alerts when these files are
       added, modified or deleted. Specifically, these tests will check that FIM is able to monitor Windows system
       folders. FIM can redirect %WINDIR%/Sysnative monitoring toward System32 folder, so the tests also check that
       when monitoring Sysnative the path is converted to system32 and events are generated there properly.

components:
    - fim

suite: windows_system_folder_redirection

targets:
    - agent

daemons:
    - wazuh-syscheckd

os_platform:
    - windows

os_version:
    - Windows 10
    - Windows Server 2019
    - Windows Server 2016

references:
    - https://documentation.wazuh.com/current/user-manual/capabilities/file-integrity/index.html

pytest_args:
    - fim_mode:
        scheduled: File monitoring is done after every configured interval elapses.
        realtime: Enable real-time monitoring on Linux (using the 'inotify' system calls) and Windows systems.
        whodata: Implies real-time monitoring but adding the 'who-data' information.
    - tier:
        0: Only level 0 tests are performed, they check basic functionalities and are quick to perform.
        1: Only level 1 tests are performed, they check functionalities of medium complexity.
        2: Only level 2 tests are performed, they check advanced functionalities and are slow to perform.

tags:
    - windows_folder_redirection
'''
from pathlib import Path

import os

import pytest
from wazuh_testing.constants.paths.logs import WAZUH_LOG_PATH
from wazuh_testing.tools.monitors.file_monitor import FileMonitor
from wazuh_testing.utils.configuration import get_test_cases_data, load_configuration_template
from wazuh_testing.utils.callbacks import generate_callback
from wazuh_testing.utils import file
from wazuh_testing.modules.fim.patterns import EVENT_TYPE_ADDED, WIN_CONVERT_FOLDER
from wazuh_testing.modules.agentd.configuration import AGENTD_WINDOWS_DEBUG
from wazuh_testing.modules.fim.configuration import SYSCHECK_DEBUG

from . import TEST_CASES_PATH, CONFIGS_PATH

# Marks
pytestmark = [pytest.mark.agent, pytest.mark.win32, pytest.mark.tier(level=1)]

# Test metadata, configuration and ids.
cases_path = Path(TEST_CASES_PATH, 'cases_windows_system_folder_redirection.yaml')
config_path = Path(CONFIGS_PATH, 'configuration_windows_system_folder_redirection.yaml')
test_configuration, test_metadata, cases_ids = get_test_cases_data(cases_path)
test_configuration = load_configuration_template(config_path, test_configuration, test_metadata)

# Set configurations required by the fixtures.
daemons_handler_configuration = {'all_daemons': True}
local_internal_options = {SYSCHECK_DEBUG: 2, AGENTD_WINDOWS_DEBUG: 2 }

# Tests
@pytest.mark.parametrize('test_configuration, test_metadata', zip(test_configuration, test_metadata), ids=cases_ids)
def test_windows_system_monitoring(test_configuration, test_metadata,configure_local_internal_options,
                             truncate_monitored_files, set_wazuh_configuration, folder_to_monitor, daemons_handler):
    '''
    description: Check if the 'wazuh-syscheckd' monitors the windows system folders (System32 and SysWOW64) properly,
    and that monitoring for Sysnative folder is redirected to System32 and works properly.

    test_phases:
        - setup:
            - Set wazuh configuration and local_internal_options.
            - Create custom folder for monitoring
            - Clean logs files and restart wazuh to apply the configuration.
        - test:
            - In case of monitoring Sysnative, check it is redirected to System32.
            - Create, Update and Delete files in monitored folders, and check logs appear.
        - teardown:
            - Delete custom monitored folder
            - Restore configuration
            - Stop wazuh

    wazuh_min_version: 4.6.0

    tier: 1

    parameters:
        - configuration:
            type: dict
            brief: Configuration values for ossec.conf.
        - metadata:
            type: dict
            brief: Test case data.
        - test_folders:
            type: dict
            brief: List of folders to be created for monitoring.
        - set_wazuh_configuration:
            type: fixture
            brief: Set ossec.conf configuration.
        - create_monitored_folders_module:
            type: fixture
            brief: Create a given list of folders when the module starts. Delete the folders at the end of the module.
        - configure_local_internal_options_function:
            type: fixture
            brief: Set local_internal_options.conf file.
        - restart_syscheck_function:
            type: fixture
            brief: restart syscheckd daemon, and truncate the ossec.log.
        - wait_syscheck_start:
            type: fixture
            brief: check that the starting FIM scan is detected.

    assertions:
        - Verify that for each modified file a 'diff' file is generated.
        - Verify that FIM events include the 'content_changes' field.
        - Verify that FIM events truncate the modifications made in a monitored file
          when it matches the 'nodiff' tag.
        - Verify that FIM events include the modifications made in a monitored file
          when it does not match the 'nodiff' tag.

    input_description: The file 'configuration_windows_system_folder_redirection.yaml' provides the configuration
                       template.
                       The file 'cases_windows_system_folder_redirection.yaml' provides the tes cases configuration
                       details for each test case.

    expected_output:
        - r'.*fim_adjust_path.*Convert '(.*) to '(.*)' to process the FIM events.'
        - r'.*Sending FIM event: (.+)$' ('added', 'modified', and 'deleted' events)'
    '''
    wazuh_log_monitor = FileMonitor(WAZUH_LOG_PATH)

    # If monitoring sysnative, check redirection log message
    if test_metadata['redirected']:
        wazuh_log_monitor.start(callback=generate_callback(WIN_CONVERT_FOLDER))
        assert wazuh_log_monitor.callback_result

    file_to_monitor = os.path.join(test_metadata['folder_to_monitor'], 'testfile')

    # Write the file
    file.write_file(file_to_monitor)
    wazuh_log_monitor.start(callback=generate_callback(EVENT_TYPE_ADDED))
