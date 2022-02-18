/* Copyright (C) 2015, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#include "registry.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <cJSON.h>

void __wrap_fim_registry_scan() {
    return;
}

cJSON* __wrap_fim_dbsync_registry_value_json_event(){
    return mock_ptr_type(cJSON*);
}

void __wrap_fim_diff_process_delete_registry(){
    function_called();
}

void __wrap_fim_diff_process_delete_value(){
    function_called();
}

void __wrap_registry_key_transaction_callback(){
    function_called();
}

void __wrap_registry_value_transaction_callback(){
    function_called();
}
