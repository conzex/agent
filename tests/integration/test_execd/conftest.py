import pytest

from wazuh_testing.constants.paths.configurations import ACTIVE_RESPONSE_CONFIGURATION
from wazuh_testing.constants.paths.logs import WAZUH_LOG_PATH
from wazuh_testing.modules.agentd.patterns import AGENTD_CONNECTED_TO_SERVER
from wazuh_testing.modules.execd.patterns import EXECD_RECEIVED_MESSAGE
from wazuh_testing.tools.file_monitor import FileMonitor
from wazuh_testing.tools.simulators import AuthdSimulator, RemotedSimulator
from wazuh_testing.utils import file
from wazuh_testing.utils.callbacks import generate_callback


@pytest.fixture()
def authd_simulator() -> AuthdSimulator:
    authd = AuthdSimulator()
    authd.start()

    yield authd

    authd.shutdown()


@pytest.fixture()
def remoted_simulator() -> RemotedSimulator:
    remoted = RemotedSimulator()
    remoted.start()

    yield remoted

    remoted.shutdown()


@pytest.fixture()
def active_response_configuration(request):
    # This fixture needs active_response_configuration to be declared.
    if not hasattr(request.module, 'active_response_configuration'):
        raise AttributeError('Error in fixture "set_active_response_configuration", '
                             'the variable active_response_configuration is not defined.')
    # Get the configuration values.
    ar_config = getattr(request.module, 'active_response_configuration')
    # Backup the ar.conf file to restore it later.
    ar_conf_exists = file.exists_and_is_file(ACTIVE_RESPONSE_CONFIGURATION)
    if ar_conf_exists:
        backup = file.read_file_lines(ACTIVE_RESPONSE_CONFIGURATION)
    # Write new Active Response configuration.
    file.write_file(ACTIVE_RESPONSE_CONFIGURATION, ar_config)

    yield

    # Restore the ar.conf file previous state.
    if ar_conf_exists:
        file.write_file(ACTIVE_RESPONSE_CONFIGURATION, backup)
    else:
        file.delete_file(ACTIVE_RESPONSE_CONFIGURATION)


@pytest.fixture()
def send_execd_message(test_metadata, authd_simulator, remoted_simulator, daemons_handler):
    monitor = FileMonitor(WAZUH_LOG_PATH)
    # Wait for agent to connect to the server.
    monitor.start(callback=generate_callback(AGENTD_CONNECTED_TO_SERVER))
    # Once the agent is 'connected' send the input
    remoted_simulator.send_custom_message(test_metadata['input'])
    # Wait for execd to start.
    monitor.start(callback=generate_callback(EXECD_RECEIVED_MESSAGE))