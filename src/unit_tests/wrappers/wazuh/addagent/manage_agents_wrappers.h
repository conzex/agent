/* Copyright (C) 2015-2021, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */


#ifndef MANAGE_AGENTS_WRAPPERS_H
#define MANAGE_AGENTS_WRAPPERS_H

int __wrap_OS_AddNewAgent(void *keys, const char *id, const char *name, const char *ip, const char *key);

void __wrap_OS_RemoveAgentGroup(const char *id);

#endif
