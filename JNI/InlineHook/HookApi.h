#pragma once
#include "../ndklog.h"

extern bool hook(void *  target_addr, void *  new_addr, void **proto_addr);
extern bool unhook(void* target_addr);
